/*****************************************************************************/
/*  Copyright (C) Brian Masney <masneyb@gftp.org>                      */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA      */
/*****************************************************************************/

/* included by options_dialog.c
 * - make_proxy_hosts_tab()
 * - new_proxy_hosts
 * 
 * The logic to add/edit localhosts needs fixing
 * - add : should not allow duplicate entries
 * - edit: should not add a new entry
 */

static GtkWidget * proxy_list, * new_proxy_domain,
                 * entry_netipv4[4], * entry_netmask[4],
                 * network_label, * netmask_label,
                 * radio_domain, * domain_label,
                 * edit_button, * delete_button,
                 * _opt_dlg;

static GList * new_proxy_hosts = NULL;

enum
{
	LOCALHTV_COL_NETWORK,
	LOCALHTV_COL_NETMASK,
	LOCALHTV_COL_HOST, /* hidden */
	LOCALHTV_NUM_COLS
};

static void make_proxy_hosts_tab (GtkWidget * notebook, GtkWidget * dialog);
static void add_proxy_host (GtkWidget * widget, gpointer data);
static void add_host_dlg_toggled_cb (GtkToggleButton *togglebutton, gpointer data);
static void delete_proxy_host (GtkWidget * widget, gpointer data);
static void ph_list_selection_cb (GtkTreeSelection * tree_sel, gpointer data);
static void add_host_dlg_response_cb (GtkDialog * dialog, gint response, gpointer user_data);
static void add_host_dlg_ok (void);
static void add_host_to_listbox (GList * templist);

// =====================================================================

static void
add_host_to_listbox (GList * templist)
{
  gftp_proxy_hosts * hosts = (gftp_proxy_hosts *) templist->data;
  char * network, * netmask;
  int free_chars = 0;
  GtkTreeIter iter;
  GtkTreeModel * model = gtk_tree_view_get_model (GTK_TREE_VIEW (proxy_list));
  GtkListStore * store = GTK_LIST_STORE (model);

  if (hosts->domain)
    {
      network = hosts->domain;
      netmask = NULL;
    }
  else
    {
      network = g_strdup_printf ("%d.%d.%d.%d",
                                 hosts->ipv4_network_address >> 24 & 0xff,
                                 hosts->ipv4_network_address >> 16 & 0xff,
                                 hosts->ipv4_network_address >> 8 & 0xff,
                                 hosts->ipv4_network_address & 0xff);
      netmask = g_strdup_printf ("%d.%d.%d.%d",
                                 hosts->ipv4_netmask >> 24 & 0xff,
                                 hosts->ipv4_netmask >> 16 & 0xff,
                                 hosts->ipv4_netmask >> 8 & 0xff,
                                 hosts->ipv4_netmask & 0xff);
      free_chars = 1;
    }

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      LOCALHTV_COL_NETWORK, network,
                      LOCALHTV_COL_NETMASK, netmask,
                      LOCALHTV_COL_HOST, templist,
                      -1);
  if (free_chars) {
    g_free (network);
    g_free (netmask);
  }
}


static void
add_host_dlg_ok (void)
{
  gftp_proxy_hosts * hosts;
  const char * edttxt;
  GList * templist = NULL;

  hosts = g_malloc0 (sizeof (*hosts));
  new_proxy_hosts = g_list_append (new_proxy_hosts, hosts);
  for (templist = new_proxy_hosts; templist->next != NULL;
       templist = templist->next);

  if (hosts->domain)
    {
      g_free (hosts->domain);
      hosts->domain = NULL;
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_domain)))
    {
      edttxt = gtk_entry_get_text (GTK_ENTRY (new_proxy_domain));
      hosts->domain = g_strdup (edttxt);
      hosts->ipv4_netmask = hosts->ipv4_network_address = 0;
    }
  else
    {
      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netipv4[0]));
      hosts->ipv4_network_address = (strtol (edttxt, NULL, 10) & 0xff) << 24;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netipv4[1]));
      hosts->ipv4_network_address |= (strtol (edttxt, NULL, 10) & 0xff) << 16;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netipv4[2]));
      hosts->ipv4_network_address |= (strtol (edttxt, NULL, 10) & 0xff) << 8;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netipv4[3]));
      hosts->ipv4_network_address |= strtol (edttxt, NULL, 10) & 0xff;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netmask[0]));
      hosts->ipv4_netmask = (strtol (edttxt, NULL, 10) & 0xff) << 24;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netmask[1]));
      hosts->ipv4_netmask |= (strtol (edttxt, NULL, 10) & 0xff) << 16;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netmask[2]));
      hosts->ipv4_netmask |= (strtol (edttxt, NULL, 10) & 0xff) << 8;

      edttxt = gtk_entry_get_text (GTK_ENTRY (entry_netmask[3]));
      hosts->ipv4_netmask |= strtol (edttxt, NULL, 10) & 0xff;
    }

  add_host_to_listbox (templist);
}


static void
add_host_dlg_response_cb (GtkDialog * dialog, gint response, gpointer user_data)
{
  if (response == GTK_RESPONSE_OK) {
     add_host_dlg_ok ();
  }
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
ph_list_selection_cb (GtkTreeSelection * tree_sel, gpointer data)
{ // enable or disable Edit/Delete buttons
  gboolean sel = FALSE;
  if (tree_sel) {
     sel = gtk_tree_selection_get_selected (tree_sel, NULL, NULL);
  }
  gtk_widget_set_sensitive (edit_button, sel);
  gtk_widget_set_sensitive (delete_button, sel);
}

static void
delete_proxy_host (GtkWidget * widget, gpointer data)
{
  GList *templist;
  GtkTreeIter iter;
  GtkTreeModel * model = gtk_tree_view_get_model (GTK_TREE_VIEW (proxy_list));
  GtkTreeSelection * sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (proxy_list));
  gftp_configuration_changed = 1; /* FIXME */

  if (!gtk_tree_selection_get_selected (sel, NULL, &iter)) {
     return;
  }
  gtk_tree_model_get (model, &iter, LOCALHTV_COL_HOST, &templist, -1);
  gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
  new_proxy_hosts = g_list_remove_link (new_proxy_hosts, templist);
  /* free templist->data (see gftp_proxy_hosts in gftp.h */
  gftp_proxy_hosts * hosts = (gftp_proxy_hosts *) templist->data;
  if (hosts->domain) g_free (hosts->domain);
  g_list_free (templist);
  /* disable edit/delete buttons */
  ph_list_selection_cb (NULL, NULL);
}

static void
add_host_dlg_toggled_cb (GtkToggleButton *togglebutton, gpointer data)
{
  gtk_widget_set_sensitive (new_proxy_domain, data != NULL);
  gtk_widget_set_sensitive (domain_label, data != NULL);
  gtk_widget_set_sensitive (network_label, data == NULL);
  gtk_widget_set_sensitive (netmask_label, data == NULL);
  int i;
  for (i = 0; i < 4; i++) {
    gtk_widget_set_sensitive (entry_netipv4[i], data == NULL);
    gtk_widget_set_sensitive (entry_netmask[i], data == NULL);
  }
}


static void
add_proxy_host (GtkWidget * widget, gpointer data)
{
  GtkWidget *tempwid, *dialog, *box, *rbox, *vbox, *radio_network, *table, *main_vbox;
  gftp_proxy_hosts *hosts;
  char *tempstr, *title;
  GList *templist;
  int i;

  gftp_configuration_changed = 1; /* FIXME */

  if (data)
    { /* Edit */
      GtkTreeModel * model = gtk_tree_view_get_model (GTK_TREE_VIEW (proxy_list));
      GtkTreeSelection * sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (proxy_list));
      GtkTreeIter iter;
      if (!gtk_tree_selection_get_selected (sel, NULL, &iter)) {
         return;
      }
      gtk_tree_model_get (model, &iter, LOCALHTV_COL_HOST, &templist, -1);
      if (!templist) {
        return;
      }
      hosts = templist->data;
    }
  else
    {
      hosts = NULL;
      templist = NULL;
    }

  title = hosts ? _("Edit Host") : _("Add Host");

  dialog = gtk_dialog_new ();

  gtk_container_set_border_width (GTK_CONTAINER (dialog), 2);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), title);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (_opt_dlg));
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_role (GTK_WINDOW(dialog), "hostinfo");

  gtk_dialog_add_button (GTK_DIALOG (dialog), "gtk-cancel", GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (dialog), "gtk-save",   GTK_RESPONSE_OK);

  main_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (main_vbox), 5);

  GtkWidget * frame = gtk_frame_new ("");
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);

  tempwid = gtk_label_new_with_mnemonic (_("_Type:"));
  gtkcompat_widget_set_halign_left (tempwid);
  gtk_box_pack_start (GTK_BOX (box), tempwid, FALSE, FALSE, 0);

  rbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (box), rbox, TRUE, TRUE, 0);

  radio_domain = gtk_radio_button_new_with_label (NULL, _("Domain"));
  g_signal_connect (G_OBJECT (radio_domain), "toggled",
                    G_CALLBACK (add_host_dlg_toggled_cb), (gpointer) 1);
  
  radio_network = gtk_radio_button_new_with_label (gtk_radio_button_get_group
                                            (GTK_RADIO_BUTTON (radio_domain)),
                                           _("Network"));
  g_signal_connect (G_OBJECT (radio_network), "toggled",
                    G_CALLBACK (add_host_dlg_toggled_cb), NULL);

  gtk_label_set_mnemonic_widget (GTK_LABEL (tempwid), radio_network);

  gtk_box_pack_start (GTK_BOX (rbox), radio_network, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (rbox), radio_domain,  FALSE, FALSE, 0);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (box), table, FALSE, FALSE, 0);

  network_label = gtk_label_new_with_mnemonic (_("_Network address:"));
  gtkcompat_widget_set_halign_left (network_label);
  gtk_table_attach_defaults (GTK_TABLE (table), network_label, 0, 1, 0, 1);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), box, 1, 2, 0, 1);

  for (i = 0; i < 4; i++) {
     entry_netipv4[i] = gtk_entry_new ();
     gtk_widget_set_size_request (entry_netipv4[i], 38, -1);
     gtk_box_pack_start (GTK_BOX (box), entry_netipv4[i], TRUE, TRUE, 0);
  }
  gtk_label_set_mnemonic_widget (GTK_LABEL (tempwid), entry_netipv4[0]);

  netmask_label = gtk_label_new_with_mnemonic (_("N_etmask:"));
  gtkcompat_widget_set_halign_left (netmask_label);
  gtk_table_attach_defaults (GTK_TABLE (table), netmask_label, 0, 1, 1, 2);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), box, 1, 2, 1, 2);

  for (i = 0; i < 4; i++) {
     entry_netmask[i] = gtk_entry_new ();
     gtk_widget_set_size_request (entry_netmask[i], 38, -1);
     gtk_box_pack_start (GTK_BOX (box), entry_netmask[i], TRUE, TRUE, 0);
  }
  gtk_label_set_mnemonic_widget (GTK_LABEL (netmask_label), entry_netmask[0]);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, TRUE, TRUE, 0);

  tempwid = gtk_label_new_with_mnemonic (_("_Domain:"));
  domain_label = tempwid;
  gtkcompat_widget_set_halign_left (tempwid);
  gtk_box_pack_start (GTK_BOX (box), tempwid, FALSE, FALSE, 0);

  new_proxy_domain = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), new_proxy_domain, TRUE, TRUE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (tempwid), new_proxy_domain);

  if (!hosts || !hosts->domain)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_network), TRUE);
      add_host_dlg_toggled_cb (NULL, NULL);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_domain), TRUE);
      add_host_dlg_toggled_cb (NULL, (gpointer) 1);
    }

  if (hosts && hosts->domain) {
     gtk_entry_set_text (GTK_ENTRY (new_proxy_domain), hosts->domain);
  } else if (hosts) {
     tempstr = g_strdup_printf ("%d", hosts->ipv4_network_address >> 24 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netipv4[0]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_network_address >> 16 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netipv4[1]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_network_address >> 8 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netipv4[2]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_network_address & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netipv4[3]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_netmask >> 24 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netmask[0]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_netmask >> 16 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netmask[1]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_netmask >> 8 & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netmask[2]), tempstr);
     g_free (tempstr);

     tempstr = g_strdup_printf ("%d", hosts->ipv4_netmask & 0xff);
     gtk_entry_set_text (GTK_ENTRY (entry_netmask[3]), tempstr);
     g_free (tempstr);
  }

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (add_host_dlg_response_cb), NULL);

  gtk_widget_show_all (dialog);
}


static void
make_proxy_hosts_tab (GtkWidget * notebook, GtkWidget * dialog)
{
  GtkWidget *tempwid, *box, *hbox, *scroll;
  gftp_config_list_vars * proxy_hosts;
  GList * templist;

  _opt_dlg = dialog;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (box), 10);

  tempwid = gtk_label_new (_("Local Hosts"));
  gtk_widget_show (tempwid);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box, tempwid);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box), scroll, TRUE, TRUE, 0);

  //-----------
  //GtkTreeView

  GtkTreeModel     *tree_model;
  GtkTreeView      *treeview;
  GtkTreeSelection *tree_sel;

  GtkListStore *store;
  store = gtk_list_store_new (LOCALHTV_NUM_COLS,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_POINTER);
  tree_model = GTK_TREE_MODEL (store);

  treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (tree_model));

  tree_sel = gtk_tree_view_get_selection (treeview);
  gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_SINGLE); //GTK_SELECTION_MULTIPLE

  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), LOCALHTV_COL_NETWORK);
  g_object_unref (tree_model);

  /* COLUMNS */
  GtkCellRenderer * renderer;
  GtkTreeViewColumn * col;

  renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 0.0, NULL);
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
                      "title",          _("Network"),
                      "resizable",      TRUE,
                      "clickable",      TRUE,
                      "sort-column-id", LOCALHTV_COL_NETWORK,
                      "sizing",         GTK_TREE_VIEW_COLUMN_AUTOSIZE,
                      NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", LOCALHTV_COL_NETWORK);
  gtk_tree_view_append_column (treeview, col);

  renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 0.0, NULL);
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
                      "title",          _("Netmask"),
                      "resizable",      TRUE,
                      "clickable",      TRUE,
                      "sort-column-id", LOCALHTV_COL_NETMASK,
                      "sizing",         GTK_TREE_VIEW_COLUMN_AUTOSIZE,
                      NULL);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", LOCALHTV_COL_NETMASK);
  gtk_tree_view_append_column (treeview, col);

  //end of GtkTreView
  //-----------------

  proxy_list = GTK_WIDGET (treeview);
  gtk_container_add (GTK_CONTAINER (scroll), proxy_list);

  gftp_lookup_global_option ("dont_use_proxy", &proxy_hosts);
  new_proxy_hosts = gftp_copy_proxy_hosts (proxy_hosts->list);

  for (templist = new_proxy_hosts;
       templist != NULL;
       templist = templist->next)
    add_host_to_listbox (templist);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_set_homogeneous (GTK_BOX(hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  tempwid = gtk_button_new_from_stock ("gtk-add");
  gtk_box_pack_start (GTK_BOX (hbox), tempwid, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (tempwid), "clicked",
                    G_CALLBACK (add_proxy_host), NULL);

  tempwid = gtk_button_new_from_stock ("gtk-edit");
  edit_button = tempwid;
  gtk_box_pack_start (GTK_BOX (hbox), tempwid, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (tempwid), "clicked",
                    G_CALLBACK (add_proxy_host), (gpointer) 1);

  tempwid = gtk_button_new_from_stock ("gtk-delete");
  delete_button = tempwid;
  gtk_box_pack_start (GTK_BOX (hbox), tempwid, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (tempwid), "clicked",
                    G_CALLBACK (delete_proxy_host), NULL);

  g_signal_connect (G_OBJECT (tree_sel), "changed",
                    G_CALLBACK (ph_list_selection_cb), NULL);

  ph_list_selection_cb (NULL, NULL);
}

