<?php
/**
 * @file
 * wmg_turnkey_onesheet.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function wmg_turnkey_onesheet_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'turnkey_onesheet';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Turnkey Onesheet';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Field: Content: Logo Image */
  $handler->display->display_options['fields']['field_logo_image']['id'] = 'field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['table'] = 'field_data_field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['field'] = 'field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['label'] = '';
  $handler->display->display_options['fields']['field_logo_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_logo_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_logo_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  /* Field: Content: LINKS - WEBSITE & SOCIALS */
  $handler->display->display_options['fields']['field_links_website_socials']['id'] = 'field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['table'] = 'field_data_field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['field'] = 'field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['label'] = '';
  $handler->display->display_options['fields']['field_links_website_socials']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_links_website_socials']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_links_website_socials']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_links_website_socials']['separator'] = '';
  /* Field: Content: VIDEO - EMBED YOUTUBE & VIMEO VIDS */
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['id'] = 'field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['table'] = 'field_data_field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['field'] = 'field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['label'] = '';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['settings'] = array(
    'file_view_mode' => 'node_gallery_file_display',
  );
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['delta_limit'] = '2';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['multi_type'] = 'ul';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['separator'] = '';
  /* Field: Content: Background Image */
  $handler->display->display_options['fields']['field_background_image']['id'] = 'field_background_image';
  $handler->display->display_options['fields']['field_background_image']['table'] = 'field_data_field_background_image';
  $handler->display->display_options['fields']['field_background_image']['field'] = 'field_background_image';
  $handler->display->display_options['fields']['field_background_image']['label'] = '';
  $handler->display->display_options['fields']['field_background_image']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_background_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_background_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_background_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: FOOTER LINKS - ADD MANAGEMENT & LABEL LOGOS / LINKS FOOTER LINKS - ADD MANAGEMENT & LABEL LOGOS / LINKS */
  $handler->display->display_options['fields']['field_footer_links']['id'] = 'field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['table'] = 'field_data_field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['field'] = 'field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['label'] = 'Footer Links';
  $handler->display->display_options['fields']['field_footer_links']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_footer_links']['alter']['text'] = '[field_footer_links-value]
[field_footer_links-revision_id]

[field_footer_links]';
  $handler->display->display_options['fields']['field_footer_links']['settings'] = array(
    'edit' => 'Edit',
    'delete' => 'Delete',
    'add' => 'Add',
    'description' => 1,
    'view_mode' => 'full',
  );
  $handler->display->display_options['fields']['field_footer_links']['delta_offset'] = '0';
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'turnkey_onesheet' => 'turnkey_onesheet',
  );

  /* Display: Block */
  $handler = $view->new_display('block', 'Block', 'block_1');
  $handler->display->display_options['defaults']['footer'] = FALSE;
  /* Footer: Global: Text area */
  $handler->display->display_options['footer']['area']['id'] = 'area';
  $handler->display->display_options['footer']['area']['table'] = 'views';
  $handler->display->display_options['footer']['area']['field'] = 'area';
  $handler->display->display_options['footer']['area']['content'] = '<style>
body.node-type-turnkey-onesheet{
background-repeat: no-repeat;
background-position: top center;
background-size:cover;
background-color: [field_background_color];
color: [field_font_color];
background-attachment: fixed;
font-size: [field_font_size];
font-family: [field_font_family] ;
}

.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-narrow .bit-events tr td a ,.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-narrow .bit-events tr.bit-bottom td a,
.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td a ,.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr.bit-bottom td a{
    color: [field_font_color];
}

body.node-type-turnkey-onesheet a{
color: [field_link_color];
}

body.node-type-turnkey-onesheet a:hover{
color: [field_hover_color];
}
body.node-type-turnkey-onesheet .bit-date a,
body.node-type-turnkey-onesheet .bit-venue a, 
body.node-type-turnkey-onesheet .bit-location a{
color: [field_link_color] !important;
}
body.node-type-turnkey-onesheet .bit-concert .venue a, body.node-type-turnkey-onesheet .bit-concert .location a,body.node-type-turnkey-onesheet .bit-concert .lineup a {
    color: [field_link_color] !important;
}

body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-date a:hover,
body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-venue a:hover,
body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-location a:hover{
color: [field_hover_color] !important;
-webkit-text-fill-color: [field_hover_color] !important;
}

#ss-bit-wide .bit-events tr td.bit-tickets a:hover {
background-color : [field_hover_color] !important;
}
body.node-type-turnkey-onesheet  #block-views-turnkey-onesheet-block-1 blockquote cite {
    display: block;
    margin-right: -3.0769em;
    text-align: center;
    width: 100%;
}

body.node-type-turnkey-onesheet #ss-bit-narrow tr td.bit-concert a.bit-buy-tix {
    background: [field_link_color];
    color: [field_background_color];
}


body.node-type-turnkey-onesheet #ss-bit-narrow tr {
    background: [field_background_color];
}

/*Styles for the soundcloud iframe player*/
.view-turnkey-onesheet .views-field-field-spotify-embed iframe[src^="https://w.soundcloud.com"] {
    max-width:600px;
    height: 150px;
}

#to-bg-img {display:none;}

.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td a {border-radius:0px !important;}

#bit-events table a {
    font-family: [field_font_family]!important;
}

.views-field-field-footer-links {margin-top: 20px;}

#ss-bit-wide .bit-events tr.bit-bottom td a, 
#ss-bit-narrow tr.bit-bottom td a,
#ss-bit-narrow td.bit-tickets a,
#ss-bit-wide .bit-events tr td.bit-tickets a {
	background-color: [field_link_color];
        color: [field_font_color];
}

/*Lightbox Change of Style*/
#cboxClose {top: 0px;}

</style>





<script type="text/javascript">
	var BITopts = {
		"artist" : "[field_bit_widget_artist_name__1]",
		"force_narrow_layout" : false,
		"display_limit" : 3,
		"bandsintown_footer_link" : false,
		"div_id" : "ss-bit-wide",
		"share_links" : false,
		"facebook_comments" : false,
		"notify_me" : false,
		"separator_color" : "transparent",
		"text_color" : "#fff",
		"link_color" : "#fff",
                "email_signup": false,
           "bg-color":"#000"
	};
	if (jQuery(\'#ss-bit-wide\').html() == \'\') {
		var BITWidget = new BIT.Widget(BITopts);
		BITWidget.insert_events();
	}

var BITopts = {
		"artist" : "[field_bit_widget_artist_name__1]",
		"force_narrow_layout" : true,
		"display_limit" : 3,
		"bandsintown_footer_link" : false,
		"div_id" : "ss-bit-narrow",
		"share_links" : false,
		"facebook_comments" : false,
		"notify_me" : false,
		"separator_color" : "transparent",
		"text_color" : "#fff",
		"link_color" : "#fff",
                "email_signup": false
	};
	if (jQuery(\'#ss-bit-narrow\').html() == \'\') {
		var BITWidget = new BIT.Widget(BITopts);
		BITWidget.insert_events();
	}

		$jq18(".views-field-field-photo-upload-up-to-3-image img").fancybox({
			openEffect : "none",
			closeEffect : "none",
			helpers : {
				title : {
					type : "inside"
				}
			},
			afterLoad : function() {
				var href = $jq18(this.element).next().attr("href");
				this.title = "<a href=\'" + href + "\'>" + this.title + "</a>";
			}
		});
/*jQuery(document).ready(function(){
	jQuery(".views-field-field-video-embed-youtube-vimeo li").each(function(){   
		$this = jQuery(this);
		var embedId = jQuery(this).find("embed").attr("src").split("/")[4].split("?")[0]; 
		var player = jQuery(\'<div class="media-embed-wrapper-outer"><div class="media-embed-wrapper"><iframe width="854" height="480" src="https://www.youtube.com/embed/\'+embedId+\'" frameborder="0" allowfullscreen></iframe></div></div>\');
		$this.append(player);	
	})
});*/

/*Setting the background image*/
var toneBGIMG = jQuery("#to-bg-img img").attr("src");
jQuery("body.node-type-turnkey-onesheet").css("background-image","url(\'" + toneBGIMG + "\')");

</script>';
  $handler->display->display_options['footer']['area']['format'] = 'script';
  $handler->display->display_options['footer']['area']['tokenize'] = TRUE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Logo Image */
  $handler->display->display_options['fields']['field_logo_image']['id'] = 'field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['table'] = 'field_data_field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['field'] = 'field_logo_image';
  $handler->display->display_options['fields']['field_logo_image']['label'] = '';
  $handler->display->display_options['fields']['field_logo_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_logo_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_logo_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  /* Field: Content: LINKS - WEBSITE & SOCIALS */
  $handler->display->display_options['fields']['field_links_website_socials']['id'] = 'field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['table'] = 'field_data_field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['field'] = 'field_links_website_socials';
  $handler->display->display_options['fields']['field_links_website_socials']['label'] = '';
  $handler->display->display_options['fields']['field_links_website_socials']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_links_website_socials']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_links_website_socials']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_links_website_socials']['separator'] = '';
  /* Field: Content: VIDEO - EMBED YOUTUBE & VIMEO VIDS */
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['id'] = 'field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['table'] = 'field_data_field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['field'] = 'field_video_embed_youtube_vimeo';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['label'] = '';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['settings'] = array(
    'file_view_mode' => 'media_original',
  );
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['multi_type'] = 'ul';
  $handler->display->display_options['fields']['field_video_embed_youtube_vimeo']['separator'] = '';
  /* Field: Content: AUDIO - SPOTIFY, SOUNDCLOUD, GENERIC EMBED */
  $handler->display->display_options['fields']['field_spotify_embed']['id'] = 'field_spotify_embed';
  $handler->display->display_options['fields']['field_spotify_embed']['table'] = 'field_data_field_spotify_embed';
  $handler->display->display_options['fields']['field_spotify_embed']['field'] = 'field_spotify_embed';
  $handler->display->display_options['fields']['field_spotify_embed']['label'] = '';
  $handler->display->display_options['fields']['field_spotify_embed']['alter']['text'] = 'iframe for you - [field_spotify_embed] ';
  $handler->display->display_options['fields']['field_spotify_embed']['alter']['max_length'] = '1000';
  $handler->display->display_options['fields']['field_spotify_embed']['alter']['trim'] = TRUE;
  $handler->display->display_options['fields']['field_spotify_embed']['alter']['html'] = TRUE;
  $handler->display->display_options['fields']['field_spotify_embed']['element_label_colon'] = FALSE;
  /* Field: Content: PHOTO - UPLOAD UP TO 3 IMAGES */
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['id'] = 'field_photo_upload_upto_3_images';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['table'] = 'field_data_field_photo_upload_upto_3_images';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['field'] = 'field_photo_upload_upto_3_images';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['label'] = '';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['type'] = 'colorbox';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['settings'] = array(
    'colorbox_node_style' => '307_307',
    'colorbox_node_style_first' => '',
    'colorbox_image_style' => '',
    'colorbox_gallery' => 'post',
    'colorbox_gallery_custom' => '',
    'colorbox_caption' => 'auto',
    'colorbox_caption_custom' => '',
  );
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['multi_type'] = 'ul';
  $handler->display->display_options['fields']['field_photo_upload_upto_3_images']['separator'] = '';
  /* Field: Content: Background Image */
  $handler->display->display_options['fields']['field_background_image']['id'] = 'field_background_image';
  $handler->display->display_options['fields']['field_background_image']['table'] = 'field_data_field_background_image';
  $handler->display->display_options['fields']['field_background_image']['field'] = 'field_background_image';
  $handler->display->display_options['fields']['field_background_image']['label'] = '';
  $handler->display->display_options['fields']['field_background_image']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_background_image']['alter']['text'] = '<div id="to-bg-img" style="display:none">[field_background_image]</div>';
  $handler->display->display_options['fields']['field_background_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_background_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_background_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: BANDS IN TOWN WIDGET ARTIST NAME */
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['id'] = 'field_bit_widget_artist_name_';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['table'] = 'field_data_field_bit_widget_artist_name_';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['field'] = 'field_bit_widget_artist_name_';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['label'] = '';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['alter']['text'] = '<div id="ss-bit-wide"></div>
<div id="ss-bit-narrow"></div>';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['type'] = 'text_plain';
  $handler->display->display_options['fields']['field_bit_widget_artist_name_']['delta_offset'] = '0';
  /* Field: Footer Links */
  $handler->display->display_options['fields']['field_footer_links']['id'] = 'field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['table'] = 'field_data_field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['field'] = 'field_footer_links';
  $handler->display->display_options['fields']['field_footer_links']['ui_name'] = 'Footer Links';
  $handler->display->display_options['fields']['field_footer_links']['label'] = '';
  $handler->display->display_options['fields']['field_footer_links']['alter']['text'] = '[field_footer_links-value]
[field_footer_links-revision_id]

[field_footer_links]';
  $handler->display->display_options['fields']['field_footer_links']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_footer_links']['type'] = 'field_collection_views_view';
  $handler->display->display_options['fields']['field_footer_links']['settings'] = array(
    'name' => 'field_collections_footer_links',
    'display_id' => 'block_1',
    'add' => 'Add',
  );
  $handler->display->display_options['fields']['field_footer_links']['delta_offset'] = '0';
  /* Field: Content: BANDS IN TOWN WIDGET ARTIST NAME */
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['id'] = 'field_bit_widget_artist_name__1';
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['table'] = 'field_data_field_bit_widget_artist_name_';
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['field'] = 'field_bit_widget_artist_name_';
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['label'] = '';
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_bit_widget_artist_name__1']['type'] = 'text_plain';
  /* Field: Content: Link Color */
  $handler->display->display_options['fields']['field_link_color']['id'] = 'field_link_color';
  $handler->display->display_options['fields']['field_link_color']['table'] = 'field_data_field_link_color';
  $handler->display->display_options['fields']['field_link_color']['field'] = 'field_link_color';
  $handler->display->display_options['fields']['field_link_color']['label'] = '';
  $handler->display->display_options['fields']['field_link_color']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_link_color']['element_label_colon'] = FALSE;
  /* Field: Content: Font Color */
  $handler->display->display_options['fields']['field_font_color']['id'] = 'field_font_color';
  $handler->display->display_options['fields']['field_font_color']['table'] = 'field_data_field_font_color';
  $handler->display->display_options['fields']['field_font_color']['field'] = 'field_font_color';
  $handler->display->display_options['fields']['field_font_color']['label'] = '';
  $handler->display->display_options['fields']['field_font_color']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_font_color']['element_label_colon'] = FALSE;
  /* Field: Content: Background Color */
  $handler->display->display_options['fields']['field_background_color']['id'] = 'field_background_color';
  $handler->display->display_options['fields']['field_background_color']['table'] = 'field_data_field_background_color';
  $handler->display->display_options['fields']['field_background_color']['field'] = 'field_background_color';
  $handler->display->display_options['fields']['field_background_color']['label'] = '';
  $handler->display->display_options['fields']['field_background_color']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_background_color']['element_label_colon'] = FALSE;
  /* Field: Content: Hover Color */
  $handler->display->display_options['fields']['field_hover_color']['id'] = 'field_hover_color';
  $handler->display->display_options['fields']['field_hover_color']['table'] = 'field_data_field_hover_color';
  $handler->display->display_options['fields']['field_hover_color']['field'] = 'field_hover_color';
  $handler->display->display_options['fields']['field_hover_color']['label'] = '';
  $handler->display->display_options['fields']['field_hover_color']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_hover_color']['element_label_colon'] = FALSE;
  /* Field: Content: Font family */
  $handler->display->display_options['fields']['field_font_family']['id'] = 'field_font_family';
  $handler->display->display_options['fields']['field_font_family']['table'] = 'field_data_field_font_family';
  $handler->display->display_options['fields']['field_font_family']['field'] = 'field_font_family';
  $handler->display->display_options['fields']['field_font_family']['label'] = '';
  $handler->display->display_options['fields']['field_font_family']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_font_family']['element_label_colon'] = FALSE;
  /* Field: Content: Font size */
  $handler->display->display_options['fields']['field_font_size']['id'] = 'field_font_size';
  $handler->display->display_options['fields']['field_font_size']['table'] = 'field_data_field_font_size';
  $handler->display->display_options['fields']['field_font_size']['field'] = 'field_font_size';
  $handler->display->display_options['fields']['field_font_size']['label'] = '';
  $handler->display->display_options['fields']['field_font_size']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_font_size']['element_label_colon'] = FALSE;
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Contextual filter: Content: Nid */
  $handler->display->display_options['arguments']['nid']['id'] = 'nid';
  $handler->display->display_options['arguments']['nid']['table'] = 'node';
  $handler->display->display_options['arguments']['nid']['field'] = 'nid';
  $handler->display->display_options['arguments']['nid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['nid']['default_argument_type'] = 'node';
  $handler->display->display_options['arguments']['nid']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['nid']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['nid']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['block_description'] = 'turnkey-onepage';
  $translatables['turnkey_onesheet'] = array(
    t('Master'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Footer Links'),
    t('[field_footer_links-value]
[field_footer_links-revision_id]

[field_footer_links]'),
    t('Block'),
    t('<style>
body.node-type-turnkey-onesheet{
background-repeat: no-repeat;
background-position: top center;
background-size:cover;
background-color: [field_background_color];
color: [field_font_color];
background-attachment: fixed;
font-size: [field_font_size];
font-family: [field_font_family] ;
}

.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-narrow .bit-events tr td a ,.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-narrow .bit-events tr.bit-bottom td a,
.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td a ,.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr.bit-bottom td a{
    color: [field_font_color];
}

body.node-type-turnkey-onesheet a{
color: [field_link_color];
}

body.node-type-turnkey-onesheet a:hover{
color: [field_hover_color];
}
body.node-type-turnkey-onesheet .bit-date a,
body.node-type-turnkey-onesheet .bit-venue a, 
body.node-type-turnkey-onesheet .bit-location a{
color: [field_link_color] !important;
}
body.node-type-turnkey-onesheet .bit-concert .venue a, body.node-type-turnkey-onesheet .bit-concert .location a,body.node-type-turnkey-onesheet .bit-concert .lineup a {
    color: [field_link_color] !important;
}

body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-date a:hover,
body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-venue a:hover,
body.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td.bit-location a:hover{
color: [field_hover_color] !important;
-webkit-text-fill-color: [field_hover_color] !important;
}

#ss-bit-wide .bit-events tr td.bit-tickets a:hover {
background-color : [field_hover_color] !important;
}
body.node-type-turnkey-onesheet  #block-views-turnkey-onesheet-block-1 blockquote cite {
    display: block;
    margin-right: -3.0769em;
    text-align: center;
    width: 100%;
}

body.node-type-turnkey-onesheet #ss-bit-narrow tr td.bit-concert a.bit-buy-tix {
    background: [field_link_color];
    color: [field_background_color];
}


body.node-type-turnkey-onesheet #ss-bit-narrow tr {
    background: [field_background_color];
}

/*Styles for the soundcloud iframe player*/
.view-turnkey-onesheet .views-field-field-spotify-embed iframe[src^="https://w.soundcloud.com"] {
    max-width:600px;
    height: 150px;
}

#to-bg-img {display:none;}

.node-type-turnkey-onesheet #block-views-turnkey-onesheet-block-1 #ss-bit-wide .bit-events tr td a {border-radius:0px !important;}

#bit-events table a {
    font-family: [field_font_family]!important;
}

.views-field-field-footer-links {margin-top: 20px;}

#ss-bit-wide .bit-events tr.bit-bottom td a, 
#ss-bit-narrow tr.bit-bottom td a,
#ss-bit-narrow td.bit-tickets a,
#ss-bit-wide .bit-events tr td.bit-tickets a {
	background-color: [field_link_color];
        color: [field_font_color];
}

/*Lightbox Change of Style*/
#cboxClose {top: 0px;}

</style>





<script type="text/javascript">
	var BITopts = {
		"artist" : "[field_bit_widget_artist_name__1]",
		"force_narrow_layout" : false,
		"display_limit" : 3,
		"bandsintown_footer_link" : false,
		"div_id" : "ss-bit-wide",
		"share_links" : false,
		"facebook_comments" : false,
		"notify_me" : false,
		"separator_color" : "transparent",
		"text_color" : "#fff",
		"link_color" : "#fff",
                "email_signup": false,
           "bg-color":"#000"
	};
	if (jQuery(\'#ss-bit-wide\').html() == \'\') {
		var BITWidget = new BIT.Widget(BITopts);
		BITWidget.insert_events();
	}

var BITopts = {
		"artist" : "[field_bit_widget_artist_name__1]",
		"force_narrow_layout" : true,
		"display_limit" : 3,
		"bandsintown_footer_link" : false,
		"div_id" : "ss-bit-narrow",
		"share_links" : false,
		"facebook_comments" : false,
		"notify_me" : false,
		"separator_color" : "transparent",
		"text_color" : "#fff",
		"link_color" : "#fff",
                "email_signup": false
	};
	if (jQuery(\'#ss-bit-narrow\').html() == \'\') {
		var BITWidget = new BIT.Widget(BITopts);
		BITWidget.insert_events();
	}

		$jq18(".views-field-field-photo-upload-up-to-3-image img").fancybox({
			openEffect : "none",
			closeEffect : "none",
			helpers : {
				title : {
					type : "inside"
				}
			},
			afterLoad : function() {
				var href = $jq18(this.element).next().attr("href");
				this.title = "<a href=\'" + href + "\'>" + this.title + "</a>";
			}
		});
/*jQuery(document).ready(function(){
	jQuery(".views-field-field-video-embed-youtube-vimeo li").each(function(){   
		$this = jQuery(this);
		var embedId = jQuery(this).find("embed").attr("src").split("/")[4].split("?")[0]; 
		var player = jQuery(\'<div class="media-embed-wrapper-outer"><div class="media-embed-wrapper"><iframe width="854" height="480" src="https://www.youtube.com/embed/\'+embedId+\'" frameborder="0" allowfullscreen></iframe></div></div>\');
		$this.append(player);	
	})
});*/

/*Setting the background image*/
var toneBGIMG = jQuery("#to-bg-img img").attr("src");
jQuery("body.node-type-turnkey-onesheet").css("background-image","url(\'" + toneBGIMG + "\')");

</script>'),
    t('iframe for you - [field_spotify_embed] '),
    t('<div id="to-bg-img" style="display:none">[field_background_image]</div>'),
    t('<div id="ss-bit-wide"></div>
<div id="ss-bit-narrow"></div>'),
    t('All'),
    t('turnkey-onepage'),
  );
  $export['turnkey_onesheet'] = $view;

  return $export;
}
