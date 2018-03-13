/**
 * @file reedah_source_edit.c  Google reader feed list source syncing support
 * 
 * Copyright (C) 2008 Arnold Noronha <arnstein87@gmail.com>
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

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>
#include "debug.h"
#include "update.h"
#include "subscription.h"
#include "common.h"
#include "feedlist.h"


#include "reedah_source.h"
#include "reedah_source_edit.h"
#include "config.h"
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <glib/gstdio.h>
#include "xml.h"

/**
 * A structure to indicate an edit to the Google Reader "database".
 * These edits are put in a queue and processed in sequential order
 * so that google does not end up processing the requests in an 
 * unintended order.
 */
typedef struct ReedahSourceAction {
	/**
	 * The guid of the item to edit. This will be ignored if the 
	 * edit is acting on an subscription rather than an item.
	 */
	gchar* guid;

	/**
	 * A MANDATORY feed url to containing the item, or the url of the 
	 * subscription to edit. 
	 */
	gchar* feedUrl;

	/**
	 * The source type. Currently known types are "feed" and "user".
	 * "user" sources are used, for example, for items that are links (as
	 * opposed to posts) in broadcast-friends. The unique id of the source
	 * is of the form <feedUrlType>/<feedUrl>.
	 */
	gchar* feedUrlType; 

	/**
	 * A callback function on completion of the edit.
	 */
	void (*callback) (ReedahSourcePtr gsource, struct ReedahSourceAction* edit, gboolean success);

	/**
	 * The type of this ReedahSourceAction.
	 */
	int actionType ; 
} *ReedahSourceActionPtr ; 

enum { 
	EDIT_ACTION_MARK_READ,
	EDIT_ACTION_MARK_UNREAD,
	EDIT_ACTION_TRACKING_MARK_UNREAD, /**< every UNREAD request, should be followed by tracking-kept-unread */
	EDIT_ACTION_MARK_STARRED,
	EDIT_ACTION_MARK_UNSTARRED,
	EDIT_ACTION_ADD_SUBSCRIPTION,
	EDIT_ACTION_REMOVE_SUBSCRIPTION
} ;
		

typedef struct ReedahSourceAction* editPtr ;

typedef struct ReedahSourceActionCtxt { 
	gchar   *nodeId ;
	ReedahSourceActionPtr action; 
} *ReedahSourceActionCtxtPtr; 


static void reedah_source_edit_push (ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean head);


static ReedahSourceActionPtr 
reedah_source_action_new (void)
{
	ReedahSourceActionPtr action = g_slice_new0 (struct ReedahSourceAction);
	return action;
}

static void 
reedah_source_action_free (ReedahSourceActionPtr action)
{ 
	g_free (action->guid);
	g_free (action->feedUrl);
	g_slice_free (struct ReedahSourceAction, action);
}

static ReedahSourceActionCtxtPtr
reedah_source_action_context_new(ReedahSourcePtr gsource, ReedahSourceActionPtr action)
{
	ReedahSourceActionCtxtPtr ctxt = g_slice_new0(struct ReedahSourceActionCtxt);
	ctxt->nodeId = g_strdup(gsource->root->id);
	ctxt->action = action;
	return ctxt;
}

static void
reedah_source_action_context_free(ReedahSourceActionCtxtPtr ctxt)
{
	g_free(ctxt->nodeId);
	g_slice_free(struct ReedahSourceActionCtxt, ctxt);
}

static void
reedah_source_edit_action_complete (const struct updateResult* const result, gpointer userdata, updateFlags flags) 
{ 
	ReedahSourceActionCtxtPtr     editCtxt = (ReedahSourceActionCtxtPtr) userdata; 
	nodePtr                       node = node_from_id (editCtxt->nodeId);
	ReedahSourcePtr               gsource; 
	ReedahSourceActionPtr         action = editCtxt->action ;
	
	reedah_source_action_context_free (editCtxt);

	if (!node) {
		reedah_source_action_free (action);
		return; /* probably got deleted before this callback */
	} 
	gsource = (ReedahSourcePtr) node->data;
		
	if (result->data == NULL || !g_str_equal (result->data, "OK")) {
		if (action->callback) 
			(*action->callback) (gsource, action, FALSE);
		debug1 (DEBUG_UPDATE, "The edit action failed with result: %s\n", result->data);
		reedah_source_action_free (action);
		return; /** @todo start a timer for next processing */
	}
	
	if (action->callback)
		action->callback (gsource, action, TRUE);

	reedah_source_action_free (action);

	/* process anything else waiting on the edit queue */
	reedah_source_edit_process (gsource);
}

/* the following reedah_source_api_* functions are simply funtions that 
   convert a ReedahSourceActionPtr to a updateRequestPtr */
 
static void
reedah_source_api_add_subscription (ReedahSourceActionPtr action, updateRequestPtr request, const gchar* token) 
{
	update_request_set_source (request, REEDAH_READER_ADD_SUBSCRIPTION_URL);
	gchar* s_escaped = g_uri_escape_string (action->feedUrl, NULL, TRUE) ;
	gchar* postdata = g_strdup_printf (REEDAH_READER_ADD_SUBSCRIPTION_POST, s_escaped, token);
	g_free (s_escaped);

	debug1 (DEBUG_UPDATE, "reedah_source: postdata [%s]", postdata);
	request->postdata = postdata ;
}

static void
reedah_source_api_remove_subscription (ReedahSourceActionPtr action, updateRequestPtr request, const gchar* token) 
{
	update_request_set_source (request, REEDAH_READER_REMOVE_SUBSCRIPTION_URL);
	gchar* s_escaped = g_uri_escape_string (action->feedUrl, NULL, TRUE);
	g_assert (!request->postdata);
	request->postdata = g_strdup_printf (REEDAH_READER_REMOVE_SUBSCRIPTION_POST, s_escaped, token);
	g_free (s_escaped);
}

static void 
reedah_source_api_edit_tag (ReedahSourceActionPtr action, updateRequestPtr request, const gchar*token) 
{
	update_request_set_source (request, REEDAH_READER_EDIT_TAG_URL); 

	const gchar* prefix = "feed" ; 
	gchar* s_escaped = g_uri_escape_string (action->feedUrl, NULL, TRUE);
	gchar* a_escaped = NULL ;
	gchar* i_escaped = g_uri_escape_string (action->guid, NULL, TRUE);
	gchar* postdata = NULL ;

	/*
	 * If the source of the item is a feed then the source *id* will be of
	 * the form tag:google.com,2005:reader/feed/http://foo.com/bar
	 * If the item is a shared link it is of the form
	 * tag:google.com,2005:reader/user/<sharer's-id>/source/com.google/link
	 * It is possible that there are items other thank link that has
	 * the ../user/.. id. The GR API requires the strings after ..:reader/
	 * while ReedahSourceAction only gives me after :reader/feed/ (or 
	 * :reader/user/ as the case might be). I therefore need to guess
	 * the prefix ('feed/' or 'user/') from just this information. 
	 */

	if (strstr(action->feedUrl, "://") == NULL) 
		prefix = "user" ;

	if (action->actionType == EDIT_ACTION_MARK_UNREAD) {
		a_escaped = g_uri_escape_string (REEDAH_READER_TAG_KEPT_UNREAD, NULL, TRUE);
		gchar *r_escaped = g_uri_escape_string (REEDAH_READER_TAG_READ, NULL, TRUE);
		postdata = g_strdup_printf (REEDAH_READER_EDIT_TAG_AR_TAG, i_escaped, prefix, s_escaped, a_escaped, r_escaped, token);
		g_free (r_escaped);
	}
	else if (action->actionType == EDIT_ACTION_MARK_READ) { 
		a_escaped = g_uri_escape_string (REEDAH_READER_TAG_READ, NULL, TRUE);
		postdata = g_strdup_printf (REEDAH_READER_EDIT_TAG_ADD_TAG, i_escaped, prefix, s_escaped, a_escaped, token);
	}
	else if (action->actionType == EDIT_ACTION_TRACKING_MARK_UNREAD) {
		a_escaped = g_uri_escape_string (REEDAH_READER_TAG_TRACKING_KEPT_UNREAD, NULL, TRUE);
		postdata = g_strdup_printf (REEDAH_READER_EDIT_TAG_ADD_TAG, i_escaped, prefix, s_escaped, a_escaped, token);
	}  
	else if (action->actionType == EDIT_ACTION_MARK_STARRED) { 
		a_escaped = g_uri_escape_string (REEDAH_READER_TAG_STARRED, NULL, TRUE) ;
		postdata = g_strdup_printf (
			REEDAH_READER_EDIT_TAG_ADD_TAG, i_escaped, prefix,
			s_escaped, a_escaped, token);
	}
	else if (action->actionType == EDIT_ACTION_MARK_UNSTARRED) {
		gchar* r_escaped = g_uri_escape_string(REEDAH_READER_TAG_STARRED, NULL, TRUE);
		postdata = g_strdup_printf (
			REEDAH_READER_EDIT_TAG_REMOVE_TAG, i_escaped, prefix,
			s_escaped, r_escaped, token);
	}
	
	else g_assert (FALSE);
	
	g_free (s_escaped);
	g_free (a_escaped); 
	g_free (i_escaped);
	
	debug1 (DEBUG_UPDATE, "reedah_source: postdata [%s]", postdata);


	request->postdata = postdata;
}

static void
reedah_source_edit_token_cb (const struct updateResult * const result, gpointer userdata, updateFlags flags)
{ 
	nodePtr          node;
	ReedahSourcePtr  gsource;
	const gchar*     token;
	ReedahSourceActionPtr          action;
	updateRequestPtr request; 

	if (result->httpstatus != 200 || result->data == NULL) { 
		/* FIXME: What is the behaviour that should go here? */
		return;
	}

	node = node_from_id ((gchar*) userdata);
	g_free (userdata);
	
	if (!node) {
		return;
	}
	gsource = (ReedahSourcePtr) node->data;


	token = result->data; 

	if (!gsource || g_queue_is_empty (gsource->actionQueue))
		return;

	action = g_queue_peek_head (gsource->actionQueue);

	request = update_request_new ();
	request->updateState = update_state_copy (gsource->root->subscription->updateState);
	request->options = update_options_copy (gsource->root->subscription->updateOptions) ;
	update_request_set_auth_value (request, gsource->authHeaderValue);

	if (action->actionType == EDIT_ACTION_MARK_READ || 
	    action->actionType == EDIT_ACTION_MARK_UNREAD || 
	    action->actionType == EDIT_ACTION_TRACKING_MARK_UNREAD ||
	    action->actionType == EDIT_ACTION_MARK_STARRED || 
	    action->actionType == EDIT_ACTION_MARK_UNSTARRED) 
		reedah_source_api_edit_tag (action, request, token);
	else if (action->actionType == EDIT_ACTION_ADD_SUBSCRIPTION ) 
		reedah_source_api_add_subscription (action, request, token);
	else if (action->actionType == EDIT_ACTION_REMOVE_SUBSCRIPTION )
		reedah_source_api_remove_subscription (action, request, token) ;

	update_execute_request (gsource, request, reedah_source_edit_action_complete, reedah_source_action_context_new(gsource, action), 0);

	action = g_queue_pop_head (gsource->actionQueue);
}

void
reedah_source_edit_process (ReedahSourcePtr gsource)
{ 
	updateRequestPtr request; 
	
	g_assert (gsource);
	if (g_queue_is_empty (gsource->actionQueue))
		return;
	
	/*
 	* Google reader has a system of tokens. So first, I need to request a 
 	* token from google, before I can make the actual edit request. The
 	* code here is the token code, the actual edit commands are in 
 	* reedah_source_edit_token_cb
	 */
	request = update_request_new ();
	request->updateState = update_state_copy (gsource->root->subscription->updateState);
	request->options = update_options_copy (gsource->root->subscription->updateOptions);
	request->source = g_strdup (REEDAH_READER_TOKEN_URL);
	update_request_set_auth_value(request, gsource->authHeaderValue);

	update_execute_request (gsource, request, reedah_source_edit_token_cb, 
	                        g_strdup(gsource->root->id), 0);
}

static void
reedah_source_edit_push_ (ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean head)
{ 
	g_assert (gsource->actionQueue);
	if (head) g_queue_push_head (gsource->actionQueue, action);
	else      g_queue_push_tail (gsource->actionQueue, action);
}

static void 
reedah_source_edit_push (ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean head)
{
	g_assert (gsource);
	nodePtr root = gsource->root;
	reedah_source_edit_push_ (gsource, action, head);

	/** @todo any flags I should specify? */
	if (gsource->loginState == REEDAH_SOURCE_STATE_NONE) 
		subscription_update(root->subscription, REEDAH_SOURCE_UPDATE_ONLY_LOGIN);
	else if ( gsource->loginState == REEDAH_SOURCE_STATE_ACTIVE) 
		reedah_source_edit_process (gsource);
}

static void 
update_read_state_callback (ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean success) 
{
	if (success) {
		// FIXME: call item_read_state_changed (item, newState);
	} else {
		debug0 (DEBUG_UPDATE, "Failed to change item state!\n");
	}
}

void
reedah_source_edit_mark_read (ReedahSourcePtr gsource, const gchar *guid, const gchar *feedUrl,	gboolean newStatus)
{
	ReedahSourceActionPtr action = reedah_source_action_new ();

	action->guid = g_strdup (guid);
	action->feedUrl = g_strdup (feedUrl);
	action->actionType = newStatus ? EDIT_ACTION_MARK_READ :
	                           EDIT_ACTION_MARK_UNREAD;
	action->callback = update_read_state_callback;
	
	reedah_source_edit_push (gsource, action, FALSE);

	if (newStatus == FALSE) { 
		/*
		 * According to the Google Reader API, to mark an item unread, 
		 * I also need to mark it as tracking-kept-unread in a separate
		 * network call.
		 */
		action = reedah_source_action_new ();
		action->guid = g_strdup (guid);
		action->feedUrl = g_strdup (feedUrl);
		action->actionType = EDIT_ACTION_TRACKING_MARK_UNREAD;
		reedah_source_edit_push (gsource, action, FALSE);
	}
}

static void
update_starred_state_callback(ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean success) 
{
	if (success) {
		// FIXME: call item_flag_changed (item, newState);
	} else {
		debug0 (DEBUG_UPDATE, "Failed to change item state!\n");
	}
}

void
reedah_source_edit_mark_starred (ReedahSourcePtr gsource, const gchar *guid, const gchar *feedUrl, gboolean newStatus)
{
	ReedahSourceActionPtr action = reedah_source_action_new ();

	action->guid = g_strdup (guid);
	action->feedUrl = g_strdup (feedUrl);
	action->actionType = newStatus ? EDIT_ACTION_MARK_STARRED : EDIT_ACTION_MARK_UNSTARRED;
	action->callback = update_starred_state_callback;
	
	reedah_source_edit_push (gsource, action, FALSE);
}

static void 
update_subscription_list_callback(ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean success) 
{
	if (success) { 
		/*
		 * It is possible that Google changed the name of the URL that
		 * was sent to it. In that case, I need to recover the URL 
		 * from the list. But a node with the old URL has already 
		 * been created. Allow the subscription update call to fix that.
		 */
		GSList* cur = gsource->root->children ;
		for(; cur; cur = g_slist_next (cur))  {
			nodePtr node = (nodePtr) cur->data ; 
			if (g_str_equal (node->subscription->source, action->feedUrl)) {
				subscription_set_source (node->subscription, "");
				feedlist_node_added (node);
			}
		}
		
		debug0 (DEBUG_UPDATE, "Subscription list was updated successful\n");
		subscription_update (gsource->root->subscription, REEDAH_SOURCE_UPDATE_ONLY_LIST);
	} else 
		debug0 (DEBUG_UPDATE, "Failed to update subscriptions\n");
}

void 
reedah_source_edit_add_subscription (ReedahSourcePtr gsource, const gchar* feedUrl)
{
	ReedahSourceActionPtr action = reedah_source_action_new () ;
	action->actionType = EDIT_ACTION_ADD_SUBSCRIPTION; 
	action->feedUrl = g_strdup (feedUrl);
	action->callback = update_subscription_list_callback;
	reedah_source_edit_push (gsource, action, TRUE);
}

static void
reedah_source_edit_remove_callback (ReedahSourcePtr gsource, ReedahSourceActionPtr action, gboolean success)
{
	if (success) {	
		/* 
		 * The node was removed from the feedlist, but could have
		 * returned because of an update before this edit request
		 * completed. No cleaner way to handle this. 
		 */
		GSList* cur = gsource->root->children ;
		for(; cur; cur = g_slist_next (cur))  {
			nodePtr node = (nodePtr) cur->data ; 
			if (g_str_equal (node->subscription->source, action->feedUrl)) {
				feedlist_node_removed (node);
				return;
			}
		}
	} else
		debug0 (DEBUG_UPDATE, "Failed to remove subscription");
}

void reedah_source_edit_remove_subscription (ReedahSourcePtr gsource, const gchar* feedUrl) 
{
	ReedahSourceActionPtr action = reedah_source_action_new (); 
	action->actionType = EDIT_ACTION_REMOVE_SUBSCRIPTION;
	action->feedUrl = g_strdup (feedUrl);
	action->callback = reedah_source_edit_remove_callback;
	reedah_source_edit_push (gsource, action, TRUE);
}

gboolean reedah_source_edit_is_in_queue (ReedahSourcePtr gsource, const gchar* guid) 
{
	/* this is inefficient, but works for the time being */
	GList *cur = gsource->actionQueue->head; 
	for(; cur; cur = g_list_next (cur)) { 
		ReedahSourceActionPtr action = cur->data; 
		if (action->guid && g_str_equal (action->guid, guid))
			return TRUE;
	}
	return FALSE;
}
