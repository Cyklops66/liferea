/**
 * @file reedah_source_feed_list.c  Reedah handling routines.
 * 
 * Copyright (C) 2013-2014  Lars Windolf <lars.lindner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "reedah_source_feed_list.h"

#include <glib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "feedlist.h"
#include "folder.h"
#include "json.h"
#include "metadata.h"
#include "node.h"
#include "subscription.h"
#include "xml.h" // FIXME

#include "fl_sources/opml_source.h"
#include "fl_sources/reedah_source.h"
#include "fl_sources/reedah_source_edit.h"

/**
 * Find a node by the source id.
 */
nodePtr
reedah_source_opml_get_node_by_source (ReedahSourcePtr gsource, const gchar *source) 
{
	return reedah_source_opml_get_subnode_by_node (gsource->root, source);
}

/**
 * Recursively find a node by the source id.
 */
nodePtr
reedah_source_opml_get_subnode_by_node (nodePtr node, const gchar *source) 
{
	nodePtr subnode;
	nodePtr subsubnode;
	GSList  *iter = node->children;
	for (; iter; iter = g_slist_next (iter)) {
		subnode = (nodePtr)iter->data;
		if (subnode->subscription
		    && g_str_equal (subnode->subscription->source, source))
			return subnode;
		else if (subnode->type->capabilities
			 & NODE_CAPABILITY_SUBFOLDERS) {
			subsubnode = reedah_source_opml_get_subnode_by_node(subnode, source);
			if (subnode != NULL)
				return subsubnode;
		}
	}
	return NULL;
}

/* subscription list merging functions */

static void
reedah_source_check_for_removal (nodePtr node, gpointer user_data)
{
	gchar	*expr = NULL;

	if (node->subscription && g_str_equal (node->subscription->source, REEDAH_READER_BROADCAST_FRIENDS_URL)) 
		return ; 

	if (IS_FEED (node)) {
		expr = g_strdup_printf ("/object/list[@name='subscriptions']/object/string[@name='id'][. = 'feed/%s']", node->subscription->source);
	} else if (IS_FOLDER (node)) {
		node_foreach_child_data (node, reedah_source_check_for_removal, user_data);
		expr = g_strdup_printf ("/object/list[@name='subscriptions']/object/list[@name='categories']/object[string='%s']", node->title);
	} else {
		g_warning ("google_opml_source_check_for_removal(): This should never happen...");
		return;
	}
	
	if (!xpath_find ((xmlNodePtr)user_data, expr)) {
		debug1 (DEBUG_UPDATE, "removing %s...", node_get_title (node));
		feedlist_node_removed (node);
	} else {
		debug1 (DEBUG_UPDATE, "keeping %s...", node_get_title (node));
	}
	g_free (expr);
}

/* 
 * Find a node by the name under root or create it.
 */
static nodePtr
reedah_source_find_or_create_folder (const gchar *name, nodePtr root)
{
	nodePtr		folder = NULL;
	GSList		*iter_parent;

	/* find a node by the name under root */
	iter_parent = root->children;
	while (iter_parent) {
		if (g_str_equal (name, node_get_title (iter_parent->data))) {
			folder = (nodePtr)iter_parent->data;
			break;
		}
		iter_parent = g_slist_next (iter_parent);
	}
	
	/* if not found, create new folder */
	if (!folder) {
		folder = node_new (folder_get_node_type ());
		node_set_title (folder, name);
		node_set_parent (folder, root, -1);
		feedlist_node_imported (folder);
		subscription_update (folder->subscription, FEED_REQ_RESET_TITLE | FEED_REQ_PRIORITY_HIGH);
	}
	
	return folder;
}

/* 
 * Check if folder of a node changed in Google Reader and move
 * node to the folder with the same name.
 */
static void
reedah_source_update_folder (xmlNodePtr match, ReedahSourcePtr gsource, nodePtr node)
{
	xmlNodePtr	xml;
	xmlChar		*label;
	const gchar	*ptitle;
	nodePtr		parent;
	
	/* check if label of a feed changed */ 
	parent = node->parent;
	ptitle = node_get_title (parent);
	xml = xpath_find (match, "./list[@name='categories']/object/string[@name='label']");
	if (xml) {
		label = xmlNodeListGetString (xml->doc,	xml->xmlChildrenNode, 1);
		if (parent == gsource->root || ! g_str_equal (label, ptitle)) {
			debug2 (DEBUG_UPDATE, "GSource feed label changed for %s to '%s'", node->id, label);
			parent = reedah_source_find_or_create_folder ((gchar*)label, gsource->root);
			node_reparent (node, parent);
		}
		xmlFree (label);
	} else {
		/* if feed has no label and parent is not gsource root, reparent to gsource root */
		if (parent != gsource->root)
			node_reparent (node, gsource->root);
	}
}

static void
reedah_source_merge_feed (ReedahSourcePtr source, const gchar *url, const gchar *title, const gchar *id)
{
	nodePtr	node;
	GSList	*iter;

	/* check if node to be merged already exists */
	iter = source->root->children;
	while (iter) {
		node = (nodePtr)iter->data;
		if (g_str_equal (node->subscription->source, url))
			return;
		iter = g_slist_next (iter);
	}

	debug2 (DEBUG_UPDATE, "adding %s (%s)", title, url);
	node = node_new (feed_get_node_type ());
	node_set_title (node, title);
	node_set_data (node, feed_new ());
		
	node_set_subscription (node, subscription_new (url, NULL, NULL));
	node->subscription->type = &reedahSourceFeedSubscriptionType;

	/* Save Reedah feed id which we need to fetch items... */
	node->subscription->metadata = metadata_list_append (node->subscription->metadata, "reedah-feed-id", id);
	db_subscription_update (node->subscription);

	node_set_parent (node, source->root, -1);
	feedlist_node_imported (node);
		
	/**
	 * @todo mark the ones as read immediately after this is done
	 * the feed as retrieved by this has the read and unread
	 * status inherently.
	 */
	subscription_update (node->subscription, FEED_REQ_RESET_TITLE | FEED_REQ_PRIORITY_HIGH);
	subscription_update_favicon (node->subscription);
}


/* OPML subscription type implementation */

static void
google_subscription_opml_cb (subscriptionPtr subscription, const struct updateResult * const result, updateFlags flags)
{
	ReedahSourcePtr	source = (ReedahSourcePtr) subscription->node->data;

	subscription->updateJob = NULL;
	
	// FIXME: the following code is very similar to ttrss!
	if (result->data && result->httpstatus == 200) {
		JsonParser	*parser = json_parser_new ();

		if (json_parser_load_from_data (parser, result->data, -1, NULL)) {
			JsonArray	*array = json_node_get_array (json_get_node (json_parser_get_root (parser), "subscriptions"));
			GList		*iter, *elements;
			GSList		*siter;
g_print(">>> %s\n",result->data);	
			/* We expect something like this:

			   [{"id":"feed\/http:\/\/rss.slashdot.org\/Slashdot\/slashdot",
                             "title":"Slashdot",
                             "categories":[],
                             "firstitemmsec":"1368112925514",
                             "htmlUrl":"null"},
                           ... 

			   Note that the data doesn't contain an URL. 
			   We recover it from the id field.
			*/
			elements = iter = json_array_get_elements (array);
			/* Add all new nodes we find */
			while (iter) {
				JsonNode *node = (JsonNode *)iter->data;
				
				/* ignore everything without a feed url */
				if (json_get_string (node, "id")) {
					reedah_source_merge_feed (source, 
					                          json_get_string (node, "id") + 5,	// FIXME: Unescape string!
					                          json_get_string (node, "title"),
					                          json_get_string (node, "id"));
				}
				iter = g_list_next (iter);
			}
			g_list_free (elements);

			/* Remove old nodes we cannot find anymore */
			siter = source->root->children;
			while (siter) {
				nodePtr node = (nodePtr)siter->data;
				gboolean found = FALSE;
				
				elements = iter = json_array_get_elements (array);
				while (iter) {
					JsonNode *json_node = (JsonNode *)iter->data;
					// FIXME: Compare with unescaped string
					if (g_str_equal (node->subscription->source, json_get_string (json_node, "id") + 5)) {
						debug1 (DEBUG_UPDATE, "node: %s", node->subscription->source);
						found = TRUE;
						break;
					}
					iter = g_list_next (iter);
				}
				g_list_free (elements);

				if (!found)			
					feedlist_node_removed (node);
				
				siter = g_slist_next (siter);
			}
			
			opml_source_export (subscription->node);	/* save new feeds to feed list */				   
			subscription->node->available = TRUE;			
			//return;
		} else {
			g_warning ("Invalid JSON returned on Reedah feed list request! >>>%s<<<", result->data);
		}

		g_object_unref (parser);
	} else {
		subscription->node->available = FALSE;
		debug0 (DEBUG_UPDATE, "reedah_subscription_cb(): ERROR: failed to get subscription list!");
	}

	if (!(flags & REEDAH_SOURCE_UPDATE_ONLY_LIST))
		node_foreach_child_data (subscription->node, node_update_subscription, GUINT_TO_POINTER (0));
}

/** functions for an efficient updating mechanism */

static void
reedah_source_opml_quick_update_helper (xmlNodePtr match, gpointer userdata) 
{
	ReedahSourcePtr gsource = (ReedahSourcePtr) userdata;
	xmlNodePtr      xmlNode;
	xmlChar         *id, *newestItemTimestamp;
	nodePtr         node = NULL; 
	const gchar     *oldNewestItemTimestamp;

	xmlNode = xpath_find (match, "./string[@name='id']");
	id = xmlNodeGetContent (xmlNode); 

	if (g_str_has_prefix (id, "feed/"))
		node = reedah_source_opml_get_node_by_source (gsource, id + strlen ("feed/"));
	else if (g_str_has_suffix (id, "broadcast-friends")) 
		node = reedah_source_opml_get_node_by_source (gsource, id);
	else {
		xmlFree (id);
		return;
	}

	if (node == NULL) {
		xmlFree (id);
		return;
	}

	xmlNode = xpath_find (match, "./number[@name='newestItemTimestampUsec']");
	newestItemTimestamp = xmlNodeGetContent (xmlNode);

	oldNewestItemTimestamp = g_hash_table_lookup (gsource->lastTimestampMap, node->subscription->source);

	if (!oldNewestItemTimestamp ||
	    (newestItemTimestamp && 
	     !g_str_equal (newestItemTimestamp, oldNewestItemTimestamp))) { 
		debug3(DEBUG_UPDATE, "ReedahSource: auto-updating %s "
		       "[oldtimestamp%s, timestamp %s]", 
		       id, oldNewestItemTimestamp, newestItemTimestamp);
		g_hash_table_insert (gsource->lastTimestampMap,
				    g_strdup (node->subscription->source), 
				    g_strdup (newestItemTimestamp));
				    
		subscription_update (node->subscription, 0);
	}

	xmlFree (newestItemTimestamp);
	xmlFree (id);
}

static void
reedah_source_opml_quick_update_cb (const struct updateResult* const result, gpointer userdata, updateFlags flags) 
{
	ReedahSourcePtr gsource = (ReedahSourcePtr) userdata;
	xmlDocPtr       doc;

	if (!result->data) { 
		/* what do I do? */
		debug0 (DEBUG_UPDATE, "ReedahSource: Unable to get unread counts, this update is aborted.");
		return;
	}
	doc = xml_parse (result->data, result->size, NULL);
	if (!doc) {
		debug0 (DEBUG_UPDATE, "ReedahSource: The XML failed to parse, maybe the session has expired. (FIXME)");
		return;
	}

	xpath_foreach_match (xmlDocGetRootElement (doc),
			    "/object/list[@name='unreadcounts']/object", 
			    reedah_source_opml_quick_update_helper, gsource);
	
	xmlFreeDoc (doc);
}

gboolean
reedah_source_opml_quick_update(ReedahSourcePtr gsource) 
{
	updateRequestPtr request = update_request_new ();
	request->updateState = update_state_copy (gsource->root->subscription->updateState);
	request->options = update_options_copy (gsource->root->subscription->updateOptions);
	update_request_set_source (request, REEDAH_READER_UNREAD_COUNTS_URL);
	update_request_set_auth_value(request, gsource->authHeaderValue);

	update_execute_request (gsource, request, reedah_source_opml_quick_update_cb,
				gsource, 0);

	return TRUE;
}


static void
reedah_source_opml_subscription_process_update_result (subscriptionPtr subscription, const struct updateResult * const result, updateFlags flags)
{
	google_subscription_opml_cb (subscription, result, flags);
}

static gboolean
reedah_source_opml_subscription_prepare_update_request (subscriptionPtr subscription, struct updateRequest *request)
{
	ReedahSourcePtr	gsource = (ReedahSourcePtr)subscription->node->data;
	
	g_assert(gsource);
	if (gsource->loginState == REEDAH_SOURCE_STATE_NONE) {
		debug0(DEBUG_UPDATE, "ReedahSource: login");
		reedah_source_login ((ReedahSourcePtr) subscription->node->data, 0) ;
		return FALSE;
	}
	debug1 (DEBUG_UPDATE, "updating Reedah subscription (node id %s)", subscription->node->id);
	
	update_request_set_source (request, REEDAH_READER_SUBSCRIPTION_LIST_URL);
	
	update_request_set_auth_value (request, gsource->authHeaderValue);
	
	return TRUE;
}

/* OPML subscription type definition */

struct subscriptionType reedahSourceOpmlSubscriptionType = {
	reedah_source_opml_subscription_prepare_update_request,
	reedah_source_opml_subscription_process_update_result
};
