<?php
/**
 * @file
 * wmg_turnkey_lightbox_toaster.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function wmg_turnkey_lightbox_toaster_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'turnkey_lightbox_toaster';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Turnkey lightbox/Toaster';
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
  $handler->display->display_options['style_plugin'] = 'list';
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
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
    'wmg_turnkey_lightbox' => 'wmg_turnkey_lightbox',
  );

  /* Display: Turnkey Lightbox JSON */
  $handler = $view->new_display('block', 'Turnkey Lightbox JSON', 'block_1');
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'views_json';
  $handler->display->display_options['style_options']['root_object'] = '';
  $handler->display->display_options['style_options']['top_child_object'] = 'parent';
  $handler->display->display_options['style_options']['plaintext_output'] = 1;
  $handler->display->display_options['style_options']['remove_newlines'] = 1;
  $handler->display->display_options['style_options']['jsonp_prefix'] = '';
  $handler->display->display_options['style_options']['content_type'] = 'text/json';
  $handler->display->display_options['style_options']['using_views_api_mode'] = 1;
  $handler->display->display_options['style_options']['object_arrays'] = 0;
  $handler->display->display_options['style_options']['numeric_strings'] = 0;
  $handler->display->display_options['style_options']['bigint_string'] = 0;
  $handler->display->display_options['style_options']['pretty_print'] = 0;
  $handler->display->display_options['style_options']['unescaped_slashes'] = 0;
  $handler->display->display_options['style_options']['unescaped_unicode'] = 0;
  $handler->display->display_options['style_options']['char_encoding'] = array(
    'JSON_HEX_QUOT' => 'JSON_HEX_QUOT',
  );
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['header'] = FALSE;
  /* Header: Global: Text area */
  $handler->display->display_options['header']['area']['id'] = 'area';
  $handler->display->display_options['header']['area']['table'] = 'views';
  $handler->display->display_options['header']['area']['field'] = 'area';
  $handler->display->display_options['header']['area']['format'] = 'script';
  $handler->display->display_options['header']['area']['tokenize'] = TRUE;
  $handler->display->display_options['defaults']['footer'] = FALSE;
  /* Footer: Global: Text area */
  $handler->display->display_options['footer']['area']['id'] = 'area';
  $handler->display->display_options['footer']['area']['table'] = 'views';
  $handler->display->display_options['footer']['area']['field'] = 'area';
  $handler->display->display_options['footer']['area']['format'] = 'script';
  $handler->display->display_options['footer']['area']['tokenize'] = TRUE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox image */
  $handler->display->display_options['fields']['field_lightbox_image']['id'] = 'field_lightbox_image';
  $handler->display->display_options['fields']['field_lightbox_image']['table'] = 'field_data_field_lightbox_image';
  $handler->display->display_options['fields']['field_lightbox_image']['field'] = 'field_lightbox_image';
  $handler->display->display_options['fields']['field_lightbox_image']['label'] = '';
  $handler->display->display_options['fields']['field_lightbox_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_lightbox_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_lightbox_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: Newsletter Id */
  $handler->display->display_options['fields']['field_newsletter_id']['id'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['table'] = 'field_data_field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['field'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['label'] = '';
  $handler->display->display_options['fields']['field_newsletter_id']['alter']['text'] = '<div class="mlist-outer-wrapper embedded-inline-list country-detect footer lightbox">
	<div class="mlist-wrapper">
		<!-- Ajax Loader Icon -->
		<div id="ajaxLoader" style="display: none">
		</div><!-- //ajaxLoader-->
		<form class="join-button-form" id="signupForm2" method="get" action="">
			<fieldset>
				<div class="mlist-header">
					Be one of the first to know about our tour dates, video premieres, and special announcements.
				</div>
				<div id="serverError" style="display: none">
					Unable to process your request at this time. Please try again later.
				</div>
				<div id="errorMsg" style="display: none">
					Please fill out the correct information.
				</div>
				<div class="mlist-field">
					<input id="email" name="email" type="email" class="m-required m-email" placeholder="Email Address"/>
				</div>
				<div class="mlist-submit">
					<input class="submit" type="submit" value="Sign Up"/>
				</div>
				<div class="primary-list-values">
					<input id="autoreply" name="autoreply" type="hidden" value="yes" />
					<input id="signuptype" name="signuptype" type="hidden" value="any" />
					<input id="Datasource" name="Datasource" type="hidden" value="[field_data_source]" />
					<input id="mainListId" name="newsletterId" type="hidden" value="[field_newsletter_id]"/>
				</div>
			</fieldset>
			<div class="termsWrapper first">
				<div class="terms">
					<a href="javascript:void(0)" class="terms"></a>
				</div>
				<div class="termsContent">
					[site_variables:terms]
				</div>
			</div>
		</form>
		<div id="thankyouBlock" style="display: none">
			<div class="thankyou-message embed" style="font-weight:bold;">
				Thank you!
			</div>
			<div id="thankyoublock-innerwrapper">
				<div class="secondFormCloseWrap">
					<a href="javascript:void(0)">x</a>
				</div>
				<div class="thankswelcome-msg" style="text-align:left;font-size:15px;">
					[site_variables:mlist_label_subscription_callout]
				</div>
				<div id ="secondary-list-values" class="secondary-list-values">
					<div id="errorLabelContainer2" style="display: none">
						Please fill out the correct information.
					</div>
					<div class="mlist-field postalcode">
						<label for="postalcode">Postal Code</label>
						<input id="postalcode" name="postalcode" type="text" autofocus/>
					</div>
					<div class="mlist-field country">
						<label for="country"> Country </label>
						<select id="country" name="country" class="m-required">
							<option value="">Select a country</option>
							<option value="United States">United States</option>
							<option value="Afghanistan">Afghanistan</option>
							<option value="Albania">Albania</option>
							<option value="Algeria">Algeria</option>
							<option value="American Samoa">American Samoa</option>
							<option value="Andorra">Andorra</option>
							<option value="Angola">Angola</option>
							<option value="Anguilla">Anguilla</option>
							<option value="Antarctica">Antarctica</option>
							<option value="Antigua and Barbuda">Antigua and Barbuda</option>
							<option value="Argentina">Argentina</option>
							<option value="Armenia">Armenia</option>
							<option value="Aruba">Aruba</option>
							<option value="Australia">Australia</option>
							<option value="Austria">Austria</option>
							<option value="Azerbaijan">Azerbaijan</option>
							<option value="Bahamas">Bahamas</option>
							<option value="Bahrain">Bahrain</option>
							<option value="Bangladesh">Bangladesh</option>
							<option value="Barbados">Barbados</option>
							<option value="Belarus">Belarus</option>
							<option value="Belgium">Belgium</option>
							<option value="Belize">Belize</option>
							<option value="Benin">Benin</option>
							<option value="Bermuda">Bermuda</option>
							<option value="Bhutan">Bhutan</option>
							<option value="Bolivia">Bolivia</option>
							<option value="Bosnia &amp; Herzegovina">Bosnia &amp; Herzegovina</option>
							<option value="Botswana">Botswana</option>
							<option value="Bouvet Island">Bouvet Island</option>
							<option value="Brazil">Brazil</option>
							<option value="British Ind Ocean Ter ">British Ind Ocean Ter </option>
							<option value="Brunei Darussalam">Brunei Darussalam</option>
							<option value="Bulgaria">Bulgaria</option>
							<option value="Burkina Faso">Burkina Faso</option>
							<option value="Burundi">Burundi</option>
							<option value="Cambodia">Cambodia</option>
							<option value="Cameroon">Cameroon</option>
							<option value="Canada">Canada</option>
							<option value="Cape Verde">Cape Verde</option>
							<option value="Cayman Islands">Cayman Islands</option>
							<option value="Central African Rep">Central African Rep</option>
							<option value="Chad">Chad</option>
							<option value="Chile">Chile</option>
							<option value="China">China</option>
							<option value="Christmas Island">Christmas Island</option>
							<option value="Cocos (Keeling Is)">Cocos (Keeling Is)</option>
							<option value="Colombia">Colombia</option>
							<option value="Comoros">Comoros</option>
							<option value="Congo">Congo</option>
							<option value="Cook Islands">Cook Islands</option>
							<option value="Costa Rica">Costa Rica</option>
							<option value="Cote D\'ivoire">Cote D\'Ivoire </option>
							<option value="Croatia (Hrvatska)">Croatia (Hrvatska)</option>
							<option value="Cuba">Cuba</option>
							<option value="Cyprus">Cyprus</option>
							<option value="Czech Republic">Czech Republic</option>
							<option value="Denmark">Denmark</option>
							<option value="Djibouti">Djibouti</option>
							<option value="Dominica">Dominica</option>
							<option value="Dominican Republic">Dominican Republic</option>
							<option value="East Timor">East Timor</option>
							<option value="Ecuador">Ecuador</option>
							<option value="Egypt">Egypt</option>
							<option value="El Salvador">El Salvador</option>
							<option value="Equatorial Guinea">Equatorial Guinea</option>
							<option value="Eritrea">Eritrea</option>
							<option value="Estonia">Estonia</option>
							<option value="Ethiopia">Ethiopia</option>
							<option value="Falkland Islands ">Falkland Islands </option>
							<option value="Faroe Islands">Faroe Islands</option>
							<option value="Fiji">Fiji</option>
							<option value="Finland">Finland</option>
							<option value="France">France</option><option value="France, Metro">France, Metro</option><option value="French Guiana">French Guiana</option><option value="French Polynesia">French Polynesia</option><option value="French Southern Ter">French Southern Ter</option><option value="Gabon">Gabon</option><option value="Gambia">Gambia</option><option value="Georgia">Georgia</option><option value="Georgia and S. Sand Is">Georgia and S. Sand Is</option><option value="Germany">Germany</option><option value="Ghana">Ghana</option><option value="Gibraltar">Gibraltar</option><option value="Greece">Greece</option><option value="Greenland">Greenland</option><option value="Grenada">Grenada</option><option value="Guadeloupe">Guadeloupe</option><option value="Guam">Guam</option><option value="Guatemala">Guatemala</option><option value="Guinea">Guinea</option><option value="Guinea-Bissau">Guinea-Bissau</option><option value="Guyana">Guyana</option><option value="Haiti">Haiti</option><option value="Heard &amp; McDonald Is">Heard &amp; McDonald Is</option><option value="Honduras">Honduras</option><option value="Hong Kong">Hong Kong</option><option value="Hungary">Hungary</option><option value="Iceland">Iceland</option><option value="India">India</option><option value="Indonesia">Indonesia</option><option value="Iran">Iran</option><option value="Iraq">Iraq</option><option value="Ireland">Ireland</option><option value="Israel">Israel</option><option value="Italy">Italy</option><option value="Jamaica">Jamaica</option><option value="Japan">Japan</option><option value="Jordan">Jordan</option><option value="Kazakhstan">Kazakhstan</option><option value="Kenya">Kenya</option><option value="Kiribati">Kiribati</option><option value="Korea (North) ">Korea (North) </option><option value="Korea (South)">Korea (South)</option><option value="Kuwait">Kuwait</option><option value="Kyrgyzstan ">Kyrgyzstan </option><option value="Laos">Laos</option><option value="Latvia">Latvia</option><option value="Lebanon">Lebanon</option><option value="Lesotho">Lesotho</option><option value="Liberia">Liberia</option><option value="Libya">Libya</option><option value="Liechtenstein">Liechtenstein</option><option value="Lithuania">Lithuania</option><option value="Luxembourg">Luxembourg</option><option value="Macau">Macau</option><option value="Macedonia">Macedonia</option><option value="Madagascar">Madagascar</option><option value="Malawi">Malawi</option><option value="Malaysia">Malaysia</option><option value="Maldives">Maldives</option><option value="Mali">Mali</option><option value="Malta">Malta</option><option value="Marshall Islands">Marshall Islands</option><option value="Martinique">Martinique</option><option value="Mauritania">Mauritania</option><option value="Mauritius">Mauritius</option><option value="Mayotte">Mayotte</option><option value="Mexico">Mexico</option><option value="Micronesia">Micronesia</option><option value="Moldova">Moldova</option><option value="Monaco">Monaco</option><option value="Mongolia">Mongolia</option><option value="Montserrat">Montserrat</option><option value="Morocco">Morocco</option><option value="Mozambique">Mozambique</option><option value="Myanmar">Myanmar</option><option value="Namibia">Namibia</option><option value="Nauru">Nauru</option><option value="Nepal">Nepal</option><option value="Netherlands">Netherlands</option><option value="Netherlands Antilles">Netherlands Antilles</option><option value="Neutral Zone Saudi/Iraq">Neutral Zone Saudi/Iraq</option><option value="New Caledonia">New Caledonia</option><option value="New Zealand">New Zealand</option><option value="Nicaragua">Nicaragua</option><option value="Niger">Niger</option><option value="Nigeria">Nigeria</option><option value="Niue">Niue</option><option value="Norfolk Island">Norfolk Island</option><option value="Northern Mariana Is">Northern Mariana Is</option><option value="Norway">Norway</option><option value="Oman">Oman</option><option value="Pakistan">Pakistan</option><option value="Palau">Palau</option><option value="Panama">Panama</option><option value="Papua New Guinea">Papua New Guinea</option><option value="Paraguay">Paraguay</option><option value="Peru">Peru</option><option value="Philippines">Philippines</option><option value="Pitcairn">Pitcairn</option><option value="Poland">Poland</option><option value="Portugal">Portugal</option><option value="Puerto Rico">Puerto Rico</option><option value="Qatar">Qatar</option><option value="Reunion">Reunion</option><option value="Romania">Romania</option><option value="Russian Federation">Russian Federation</option><option value="Rwanda">Rwanda</option><option value="Saint Kitts and Nevis">Saint Kitts and Nevis</option><option value="Saint Lucia">Saint Lucia</option><option value="St. Vincent/Grenadines ">St. Vincent/Grenadines </option><option value="Samoa">Samoa</option><option value="San Marino">San Marino</option><option value="Sao Tome and Principe">Sao Tome and Principe</option><option value="Saudi Arabia">Saudi Arabia</option><option value="Senegal">Senegal</option><option value="Seychelles">Seychelles</option><option value="Sierra Leone">Sierra Leone</option><option value="Singapore">Singapore</option><option value="Slovakia (Slovak Rep)">Slovakia (Slovak Rep)</option><option value="Slovenia">Slovenia</option><option value="Solomon Islands">Solomon Islands</option><option value="Somalia">Somalia</option><option value="South Africa">South Africa</option><option value="Soviet Union (former)">Soviet Union (former)</option><option value="Spain">Spain</option><option value="Sri Lanka">Sri Lanka</option><option value="St. Helena">St. Helena</option><option value="St. Pierre and Miquelo ">St. Pierre and Miquelo </option><option value="Sudan">Sudan</option><option value="Suriname">Suriname</option><option value="Svalbard &amp; Jan Mayen">Svalbard &amp; Jan Mayen</option><option value="Swaziland">Swaziland</option><option value="Sweden">Sweden</option><option value="Switzerland">Switzerland</option><option value="Syria">Syria</option><option value="Taiwan">Taiwan</option><option value="Tajikistan">Tajikistan</option><option value="Tanzania">Tanzania</option><option value="Thailand">Thailand</option><option value="Togo">Togo</option><option value="Tokelau">Tokelau</option><option value="Tonga">Tonga</option><option value="Trinidad and Tobago">Trinidad and Tobago</option><option value="Tunisia">Tunisia</option><option value="Turkey">Turkey</option><option value="Turkmenistan">Turkmenistan</option><option value="Turks and Caicos Is">Turks and Caicos Is</option><option value="Tuvalu">Tuvalu</option><option value="US Minor Outlying Is">US Minor Outlying Is</option><option value="Uganda">Uganda</option><option value="Ukraine">Ukraine</option><option value="United Arab Emirates">United Arab Emirates</option><option value="United Kingdom">United Kingdom </option><option value="Uruguay">Uruguay</option><option value="Uzbekistan">Uzbekistan</option><option value="Vanuatu">Vanuatu</option><option value="Vatican City State ">Vatican City State </option><option value="Venezuela">Venezuela</option><option value="Viet Nam">Viet Nam</option><option value="Virgin Islands (Brit)">Virgin Islands (Brit)</option><option value="Virgin Islands (US)">Virgin Islands (US)</option><option value="Wallis and Futuna Is">Wallis and Futuna Is</option><option value="Western Sahara">Western Sahara</option><option value="Yemen">Yemen</option><option value="Yugoslavia">Yugoslavia</option><option value="Zaire">Zaire</option><option value="Zambia">Zambia</option><option value="Zimbabwe">Zimbabwe</option>
						</select>
					</div>
					<div class="mlist-field mlist-field1">
						<label for="firstname">First name</label>
						<input id="firstname" name="firstname" type="text" />
					</div>
					<div class="mlist-field">
						<label for="birthday">Birth Date</label>
						<select name="dobMonth" id="dobMonth">
							<option value="">Month</option><option value="1">January</option><option value="2">February</option><option value="3">March</option><option value="4">April</option><option value="5">May</option><option value="6">June</option><option value="7">July</option><option value="8">August</option><option value="9">September</option><option value="10">October</option><option value="11">November</option><option value="12">December</option>
						</select>
						<select name="dobDay" id="dobDay">
							<option value="">Day</option><option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option><option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8">8</option><option value="9">9</option><option value="10">10</option><option value="11">11</option><option value="12">12</option><option value="13">13</option><option value="14">14</option><option value="15">15</option><option value="16">16</option><option value="17">17</option><option value="18">18</option><option value="19">19</option><option value="20">20</option><option value="21">21</option><option value="22">22</option><option value="23">23</option><option value="24">24</option><option value="25">25</option><option value="26">26</option><option value="27">27</option><option value="28">28</option><option value="29">29</option><option value="30">30</option><option value="31">31</option>
						</select>
					</div>
					<input id="mainListId" name="newsletterId" type="hidden" value="14073863"/>
					<input id="Datasource" name="Datasource" type="hidden" value="[Artist Name]_website_newsletter_optin" />
					<div class="mlist-checks">
						<input type="checkbox" class="global-list-checkbox" name="newsletterId" value="[field_label_list_id]"/>
						<label class="global-list" for="global-list">[site_variables:mlist_global_optin_language]</label>
					</div>
				</div>
				<div class="mlist-submit-new">
					<input class="submit" type="submit" value="Submit"/>
				</div>
				<div class="termsWrapper second">
					<div class="terms">
						<a href="javascript:void(0)" class="terms"></a>
					</div>
					<div class="termsContent">
						[site_variables:terms]
					</div>
				</div>
			</div>
		</div><!-- //Thank you block -->
		<!--terms -->
	</div>
</div>';
  $handler->display->display_options['fields']['field_newsletter_id']['element_label_colon'] = FALSE;
  /* Field: Content: Label List Id */
  $handler->display->display_options['fields']['field_label_list_id']['id'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['table'] = 'field_data_field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['field'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['label'] = '';
  $handler->display->display_options['fields']['field_label_list_id']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rate']['id'] = 'field_display_rate';
  $handler->display->display_options['fields']['field_display_rate']['table'] = 'field_data_field_display_rate';
  $handler->display->display_options['fields']['field_display_rate']['field'] = 'field_display_rate';
  $handler->display->display_options['fields']['field_display_rate']['label'] = '';
  $handler->display->display_options['fields']['field_display_rate']['element_label_colon'] = FALSE;
  /* Field: Content: Merch Item Name */
  $handler->display->display_options['fields']['field_merch_item_name']['id'] = 'field_merch_item_name';
  $handler->display->display_options['fields']['field_merch_item_name']['table'] = 'field_data_field_merch_item_name';
  $handler->display->display_options['fields']['field_merch_item_name']['field'] = 'field_merch_item_name';
  $handler->display->display_options['fields']['field_merch_item_name']['label'] = '';
  $handler->display->display_options['fields']['field_merch_item_name']['alter']['text'] = '<div class="merchWrapper">
  <div class="merchName">[field_merch_item_name]</div>
  <div class="merchDesc">[body]</div>
  <div class="merchPrice">[field_merch_item_price]</div>
  <div class="merchLink"><a href="[field_merch_item_link]">Buy</a></div>
</div>';
  $handler->display->display_options['fields']['field_merch_item_name']['element_label_colon'] = FALSE;
  /* Field: Content: Merch Item Price */
  $handler->display->display_options['fields']['field_merch_item_price']['id'] = 'field_merch_item_price';
  $handler->display->display_options['fields']['field_merch_item_price']['table'] = 'field_data_field_merch_item_price';
  $handler->display->display_options['fields']['field_merch_item_price']['field'] = 'field_merch_item_price';
  $handler->display->display_options['fields']['field_merch_item_price']['label'] = '';
  $handler->display->display_options['fields']['field_merch_item_price']['element_label_colon'] = FALSE;
  /* Field: Content: Merch Item Link */
  $handler->display->display_options['fields']['field_merch_item_link']['id'] = 'field_merch_item_link';
  $handler->display->display_options['fields']['field_merch_item_link']['table'] = 'field_data_field_merch_item_link';
  $handler->display->display_options['fields']['field_merch_item_link']['field'] = 'field_merch_item_link';
  $handler->display->display_options['fields']['field_merch_item_link']['label'] = '';
  $handler->display->display_options['fields']['field_merch_item_link']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_merch_item_link']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_merch_item_link']['type'] = 'link_plain';
  $handler->display->display_options['fields']['field_merch_item_link']['group_column'] = 'value';
  /* Field: Content: Generic Call To Action button  */
  $handler->display->display_options['fields']['field_cta_button']['id'] = 'field_cta_button';
  $handler->display->display_options['fields']['field_cta_button']['table'] = 'field_data_field_cta_button';
  $handler->display->display_options['fields']['field_cta_button']['field'] = 'field_cta_button';
  $handler->display->display_options['fields']['field_cta_button']['label'] = '';
  $handler->display->display_options['fields']['field_cta_button']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_cta_button']['alter']['text'] = '"[field_cta_button-title]":"[field_cta_button-url]"';
  $handler->display->display_options['fields']['field_cta_button']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_cta_button']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_cta_button']['type'] = 'link_title_plain';
  $handler->display->display_options['fields']['field_cta_button']['delta_offset'] = '0';
  $handler->display->display_options['fields']['field_cta_button']['separator'] = ',';
  /* Field: Content: Generic Call To Action button  */
  $handler->display->display_options['fields']['field_cta_button_1']['id'] = 'field_cta_button_1';
  $handler->display->display_options['fields']['field_cta_button_1']['table'] = 'field_data_field_cta_button';
  $handler->display->display_options['fields']['field_cta_button_1']['field'] = 'field_cta_button';
  $handler->display->display_options['fields']['field_cta_button_1']['label'] = '';
  $handler->display->display_options['fields']['field_cta_button_1']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_cta_button_1']['alter']['text'] = '{[field_cta_button]}';
  $handler->display->display_options['fields']['field_cta_button_1']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_cta_button_1']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_cta_button_1']['type'] = 'link_title_plain';
  $handler->display->display_options['fields']['field_cta_button_1']['delta_limit'] = '1';
  $handler->display->display_options['fields']['field_cta_button_1']['delta_offset'] = '0';
  /* Field: Content: Preview mode */
  $handler->display->display_options['fields']['field_preview_mode']['id'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['table'] = 'field_data_field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['field'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['label'] = '';
  $handler->display->display_options['fields']['field_preview_mode']['element_label_colon'] = FALSE;
  /* Field: Content: Merch Item Link */
  $handler->display->display_options['fields']['field_merch_item_link_1']['id'] = 'field_merch_item_link_1';
  $handler->display->display_options['fields']['field_merch_item_link_1']['table'] = 'field_data_field_merch_item_link';
  $handler->display->display_options['fields']['field_merch_item_link_1']['field'] = 'field_merch_item_link';
  $handler->display->display_options['fields']['field_merch_item_link_1']['label'] = '';
  $handler->display->display_options['fields']['field_merch_item_link_1']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_merch_item_link_1']['click_sort_column'] = 'url';
  $handler->display->display_options['fields']['field_merch_item_link_1']['type'] = 'link_title_plain';
  /* Field: Content: Lightbox Height */
  $handler->display->display_options['fields']['field_lbox_height']['id'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['table'] = 'field_data_field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['field'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_height']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox Width */
  $handler->display->display_options['fields']['field_lbox_width']['id'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['table'] = 'field_data_field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['field'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_width']['element_label_colon'] = FALSE;
  /* Field: Content: Mailing List Title */
  $handler->display->display_options['fields']['field_mailing_list_title']['id'] = 'field_mailing_list_title';
  $handler->display->display_options['fields']['field_mailing_list_title']['table'] = 'field_data_field_mailing_list_title';
  $handler->display->display_options['fields']['field_mailing_list_title']['field'] = 'field_mailing_list_title';
  $handler->display->display_options['fields']['field_mailing_list_title']['label'] = '';
  $handler->display->display_options['fields']['field_mailing_list_title']['element_label_colon'] = FALSE;
  /* Field: Content: Data ext */
  $handler->display->display_options['fields']['field_data_ext']['id'] = 'field_data_ext';
  $handler->display->display_options['fields']['field_data_ext']['table'] = 'field_data_field_data_ext';
  $handler->display->display_options['fields']['field_data_ext']['field'] = 'field_data_ext';
  $handler->display->display_options['fields']['field_data_ext']['label'] = '';
  $handler->display->display_options['fields']['field_data_ext']['element_label_colon'] = FALSE;
  /* Field: Content: Trigger Id */
  $handler->display->display_options['fields']['field_trigger_id']['id'] = 'field_trigger_id';
  $handler->display->display_options['fields']['field_trigger_id']['table'] = 'field_data_field_trigger_id';
  $handler->display->display_options['fields']['field_trigger_id']['field'] = 'field_trigger_id';
  $handler->display->display_options['fields']['field_trigger_id']['label'] = '';
  $handler->display->display_options['fields']['field_trigger_id']['element_label_colon'] = FALSE;
  $handler->display->display_options['defaults']['sorts'] = FALSE;
  /* Sort criterion: Content: Updated date */
  $handler->display->display_options['sorts']['changed']['id'] = 'changed';
  $handler->display->display_options['sorts']['changed']['table'] = 'node';
  $handler->display->display_options['sorts']['changed']['field'] = 'changed';
  $handler->display->display_options['sorts']['changed']['order'] = 'DESC';
  $handler->display->display_options['block_description'] = 'Turnkey-Lightbox-JSON';

  /* Display: Turnkey Toaster JSON */
  $handler = $view->new_display('block', 'Turnkey Toaster JSON', 'block_2');
  $handler->display->display_options['display_description'] = 'JSON values for the Toaster content';
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'views_json';
  $handler->display->display_options['style_options']['root_object'] = '';
  $handler->display->display_options['style_options']['top_child_object'] = 'parent';
  $handler->display->display_options['style_options']['plaintext_output'] = 1;
  $handler->display->display_options['style_options']['remove_newlines'] = 1;
  $handler->display->display_options['style_options']['jsonp_prefix'] = '';
  $handler->display->display_options['style_options']['content_type'] = 'text/json';
  $handler->display->display_options['style_options']['using_views_api_mode'] = 1;
  $handler->display->display_options['style_options']['object_arrays'] = 0;
  $handler->display->display_options['style_options']['numeric_strings'] = 0;
  $handler->display->display_options['style_options']['bigint_string'] = 0;
  $handler->display->display_options['style_options']['pretty_print'] = 0;
  $handler->display->display_options['style_options']['unescaped_slashes'] = 0;
  $handler->display->display_options['style_options']['unescaped_unicode'] = 0;
  $handler->display->display_options['style_options']['char_encoding'] = array(
    'JSON_HEX_QUOT' => 'JSON_HEX_QUOT',
  );
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['header'] = FALSE;
  /* Header: Global: Text area */
  $handler->display->display_options['header']['area']['id'] = 'area';
  $handler->display->display_options['header']['area']['table'] = 'views';
  $handler->display->display_options['header']['area']['field'] = 'area';
  $handler->display->display_options['header']['area']['format'] = 'script';
  $handler->display->display_options['header']['area']['tokenize'] = TRUE;
  $handler->display->display_options['defaults']['footer'] = FALSE;
  /* Footer: Global: Text area */
  $handler->display->display_options['footer']['area']['id'] = 'area';
  $handler->display->display_options['footer']['area']['table'] = 'views';
  $handler->display->display_options['footer']['area']['field'] = 'area';
  $handler->display->display_options['footer']['area']['format'] = 'script';
  $handler->display->display_options['footer']['area']['tokenize'] = TRUE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: Newsletter Id */
  $handler->display->display_options['fields']['field_newsletter_id']['id'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['table'] = 'field_data_field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['field'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['label'] = '';
  $handler->display->display_options['fields']['field_newsletter_id']['alter']['text'] = '<div class="mlist-outer-wrapper embedded-inline-list country-detect footer lightbox">
	<div class="mlist-wrapper">
		<!-- Ajax Loader Icon -->
		<div id="ajaxLoader" style="display: none">
		</div><!-- //ajaxLoader-->
		<form class="join-button-form" id="signupForm2" method="get" action="">
			<fieldset>
				<div class="mlist-header">
					Be one of the first to know about our tour dates, video premieres, and special announcements.
				</div>
				<div id="serverError" style="display: none">
					Unable to process your request at this time. Please try again later.
				</div>
				<div id="errorMsg" style="display: none">
					Please fill out the correct information.
				</div>
				<div class="mlist-field">
					<input id="email" name="email" type="email" class="m-required m-email" placeholder="Email Address"/>
				</div>
				<div class="mlist-submit">
					<input class="submit" type="submit" value="Sign Up"/>
				</div>
				<div class="primary-list-values">
					<input id="autoreply" name="autoreply" type="hidden" value="yes" />
					<input id="signuptype" name="signuptype" type="hidden" value="any" />
					<input id="Datasource" name="Datasource" type="hidden" value="[field_data_source]" />
					<input id="mainListId" name="newsletterId" type="hidden" value="[field_newsletter_id]"/>
				</div>
			</fieldset>
			<div class="termsWrapper first">
				<div class="terms">
					<a href="javascript:void(0)" class="terms"></a>
				</div>
				<div class="termsContent">
					[site_variables:terms]
				</div>
			</div>
		</form>
		<div id="thankyouBlock" style="display: none">
			<div class="thankyou-message embed" style="font-weight:bold;">
				Thank you!
			</div>
			<div id="thankyoublock-innerwrapper">
				<div class="secondFormCloseWrap">
					<a href="javascript:void(0)">x</a>
				</div>
				<div class="thankswelcome-msg" style="text-align:left;font-size:15px;">
					[site_variables:mlist_label_subscription_callout]
				</div>
				<div id ="secondary-list-values" class="secondary-list-values">
					<div id="errorLabelContainer2" style="display: none">
						Please fill out the correct information.
					</div>
					<div class="mlist-field postalcode">
						<label for="postalcode">Postal Code</label>
						<input id="postalcode" name="postalcode" type="text" autofocus/>
					</div>
					<div class="mlist-field country">
						<label for="country"> Country </label>
						<select id="country" name="country" class="m-required">
							<option value="">Select a country</option>
							<option value="United States">United States</option>
							<option value="Afghanistan">Afghanistan</option>
							<option value="Albania">Albania</option>
							<option value="Algeria">Algeria</option>
							<option value="American Samoa">American Samoa</option>
							<option value="Andorra">Andorra</option>
							<option value="Angola">Angola</option>
							<option value="Anguilla">Anguilla</option>
							<option value="Antarctica">Antarctica</option>
							<option value="Antigua and Barbuda">Antigua and Barbuda</option>
							<option value="Argentina">Argentina</option>
							<option value="Armenia">Armenia</option>
							<option value="Aruba">Aruba</option>
							<option value="Australia">Australia</option>
							<option value="Austria">Austria</option>
							<option value="Azerbaijan">Azerbaijan</option>
							<option value="Bahamas">Bahamas</option>
							<option value="Bahrain">Bahrain</option>
							<option value="Bangladesh">Bangladesh</option>
							<option value="Barbados">Barbados</option>
							<option value="Belarus">Belarus</option>
							<option value="Belgium">Belgium</option>
							<option value="Belize">Belize</option>
							<option value="Benin">Benin</option>
							<option value="Bermuda">Bermuda</option>
							<option value="Bhutan">Bhutan</option>
							<option value="Bolivia">Bolivia</option>
							<option value="Bosnia &amp; Herzegovina">Bosnia &amp; Herzegovina</option>
							<option value="Botswana">Botswana</option>
							<option value="Bouvet Island">Bouvet Island</option>
							<option value="Brazil">Brazil</option>
							<option value="British Ind Ocean Ter ">British Ind Ocean Ter </option>
							<option value="Brunei Darussalam">Brunei Darussalam</option>
							<option value="Bulgaria">Bulgaria</option>
							<option value="Burkina Faso">Burkina Faso</option>
							<option value="Burundi">Burundi</option>
							<option value="Cambodia">Cambodia</option>
							<option value="Cameroon">Cameroon</option>
							<option value="Canada">Canada</option>
							<option value="Cape Verde">Cape Verde</option>
							<option value="Cayman Islands">Cayman Islands</option>
							<option value="Central African Rep">Central African Rep</option>
							<option value="Chad">Chad</option>
							<option value="Chile">Chile</option>
							<option value="China">China</option>
							<option value="Christmas Island">Christmas Island</option>
							<option value="Cocos (Keeling Is)">Cocos (Keeling Is)</option>
							<option value="Colombia">Colombia</option>
							<option value="Comoros">Comoros</option>
							<option value="Congo">Congo</option>
							<option value="Cook Islands">Cook Islands</option>
							<option value="Costa Rica">Costa Rica</option>
							<option value="Cote D\'ivoire">Cote D\'Ivoire </option>
							<option value="Croatia (Hrvatska)">Croatia (Hrvatska)</option>
							<option value="Cuba">Cuba</option>
							<option value="Cyprus">Cyprus</option>
							<option value="Czech Republic">Czech Republic</option>
							<option value="Denmark">Denmark</option>
							<option value="Djibouti">Djibouti</option>
							<option value="Dominica">Dominica</option>
							<option value="Dominican Republic">Dominican Republic</option>
							<option value="East Timor">East Timor</option>
							<option value="Ecuador">Ecuador</option>
							<option value="Egypt">Egypt</option>
							<option value="El Salvador">El Salvador</option>
							<option value="Equatorial Guinea">Equatorial Guinea</option>
							<option value="Eritrea">Eritrea</option>
							<option value="Estonia">Estonia</option>
							<option value="Ethiopia">Ethiopia</option>
							<option value="Falkland Islands ">Falkland Islands </option>
							<option value="Faroe Islands">Faroe Islands</option>
							<option value="Fiji">Fiji</option>
							<option value="Finland">Finland</option>
							<option value="France">France</option><option value="France, Metro">France, Metro</option><option value="French Guiana">French Guiana</option><option value="French Polynesia">French Polynesia</option><option value="French Southern Ter">French Southern Ter</option><option value="Gabon">Gabon</option><option value="Gambia">Gambia</option><option value="Georgia">Georgia</option><option value="Georgia and S. Sand Is">Georgia and S. Sand Is</option><option value="Germany">Germany</option><option value="Ghana">Ghana</option><option value="Gibraltar">Gibraltar</option><option value="Greece">Greece</option><option value="Greenland">Greenland</option><option value="Grenada">Grenada</option><option value="Guadeloupe">Guadeloupe</option><option value="Guam">Guam</option><option value="Guatemala">Guatemala</option><option value="Guinea">Guinea</option><option value="Guinea-Bissau">Guinea-Bissau</option><option value="Guyana">Guyana</option><option value="Haiti">Haiti</option><option value="Heard &amp; McDonald Is">Heard &amp; McDonald Is</option><option value="Honduras">Honduras</option><option value="Hong Kong">Hong Kong</option><option value="Hungary">Hungary</option><option value="Iceland">Iceland</option><option value="India">India</option><option value="Indonesia">Indonesia</option><option value="Iran">Iran</option><option value="Iraq">Iraq</option><option value="Ireland">Ireland</option><option value="Israel">Israel</option><option value="Italy">Italy</option><option value="Jamaica">Jamaica</option><option value="Japan">Japan</option><option value="Jordan">Jordan</option><option value="Kazakhstan">Kazakhstan</option><option value="Kenya">Kenya</option><option value="Kiribati">Kiribati</option><option value="Korea (North) ">Korea (North) </option><option value="Korea (South)">Korea (South)</option><option value="Kuwait">Kuwait</option><option value="Kyrgyzstan ">Kyrgyzstan </option><option value="Laos">Laos</option><option value="Latvia">Latvia</option><option value="Lebanon">Lebanon</option><option value="Lesotho">Lesotho</option><option value="Liberia">Liberia</option><option value="Libya">Libya</option><option value="Liechtenstein">Liechtenstein</option><option value="Lithuania">Lithuania</option><option value="Luxembourg">Luxembourg</option><option value="Macau">Macau</option><option value="Macedonia">Macedonia</option><option value="Madagascar">Madagascar</option><option value="Malawi">Malawi</option><option value="Malaysia">Malaysia</option><option value="Maldives">Maldives</option><option value="Mali">Mali</option><option value="Malta">Malta</option><option value="Marshall Islands">Marshall Islands</option><option value="Martinique">Martinique</option><option value="Mauritania">Mauritania</option><option value="Mauritius">Mauritius</option><option value="Mayotte">Mayotte</option><option value="Mexico">Mexico</option><option value="Micronesia">Micronesia</option><option value="Moldova">Moldova</option><option value="Monaco">Monaco</option><option value="Mongolia">Mongolia</option><option value="Montserrat">Montserrat</option><option value="Morocco">Morocco</option><option value="Mozambique">Mozambique</option><option value="Myanmar">Myanmar</option><option value="Namibia">Namibia</option><option value="Nauru">Nauru</option><option value="Nepal">Nepal</option><option value="Netherlands">Netherlands</option><option value="Netherlands Antilles">Netherlands Antilles</option><option value="Neutral Zone Saudi/Iraq">Neutral Zone Saudi/Iraq</option><option value="New Caledonia">New Caledonia</option><option value="New Zealand">New Zealand</option><option value="Nicaragua">Nicaragua</option><option value="Niger">Niger</option><option value="Nigeria">Nigeria</option><option value="Niue">Niue</option><option value="Norfolk Island">Norfolk Island</option><option value="Northern Mariana Is">Northern Mariana Is</option><option value="Norway">Norway</option><option value="Oman">Oman</option><option value="Pakistan">Pakistan</option><option value="Palau">Palau</option><option value="Panama">Panama</option><option value="Papua New Guinea">Papua New Guinea</option><option value="Paraguay">Paraguay</option><option value="Peru">Peru</option><option value="Philippines">Philippines</option><option value="Pitcairn">Pitcairn</option><option value="Poland">Poland</option><option value="Portugal">Portugal</option><option value="Puerto Rico">Puerto Rico</option><option value="Qatar">Qatar</option><option value="Reunion">Reunion</option><option value="Romania">Romania</option><option value="Russian Federation">Russian Federation</option><option value="Rwanda">Rwanda</option><option value="Saint Kitts and Nevis">Saint Kitts and Nevis</option><option value="Saint Lucia">Saint Lucia</option><option value="St. Vincent/Grenadines ">St. Vincent/Grenadines </option><option value="Samoa">Samoa</option><option value="San Marino">San Marino</option><option value="Sao Tome and Principe">Sao Tome and Principe</option><option value="Saudi Arabia">Saudi Arabia</option><option value="Senegal">Senegal</option><option value="Seychelles">Seychelles</option><option value="Sierra Leone">Sierra Leone</option><option value="Singapore">Singapore</option><option value="Slovakia (Slovak Rep)">Slovakia (Slovak Rep)</option><option value="Slovenia">Slovenia</option><option value="Solomon Islands">Solomon Islands</option><option value="Somalia">Somalia</option><option value="South Africa">South Africa</option><option value="Soviet Union (former)">Soviet Union (former)</option><option value="Spain">Spain</option><option value="Sri Lanka">Sri Lanka</option><option value="St. Helena">St. Helena</option><option value="St. Pierre and Miquelo ">St. Pierre and Miquelo </option><option value="Sudan">Sudan</option><option value="Suriname">Suriname</option><option value="Svalbard &amp; Jan Mayen">Svalbard &amp; Jan Mayen</option><option value="Swaziland">Swaziland</option><option value="Sweden">Sweden</option><option value="Switzerland">Switzerland</option><option value="Syria">Syria</option><option value="Taiwan">Taiwan</option><option value="Tajikistan">Tajikistan</option><option value="Tanzania">Tanzania</option><option value="Thailand">Thailand</option><option value="Togo">Togo</option><option value="Tokelau">Tokelau</option><option value="Tonga">Tonga</option><option value="Trinidad and Tobago">Trinidad and Tobago</option><option value="Tunisia">Tunisia</option><option value="Turkey">Turkey</option><option value="Turkmenistan">Turkmenistan</option><option value="Turks and Caicos Is">Turks and Caicos Is</option><option value="Tuvalu">Tuvalu</option><option value="US Minor Outlying Is">US Minor Outlying Is</option><option value="Uganda">Uganda</option><option value="Ukraine">Ukraine</option><option value="United Arab Emirates">United Arab Emirates</option><option value="United Kingdom">United Kingdom </option><option value="Uruguay">Uruguay</option><option value="Uzbekistan">Uzbekistan</option><option value="Vanuatu">Vanuatu</option><option value="Vatican City State ">Vatican City State </option><option value="Venezuela">Venezuela</option><option value="Viet Nam">Viet Nam</option><option value="Virgin Islands (Brit)">Virgin Islands (Brit)</option><option value="Virgin Islands (US)">Virgin Islands (US)</option><option value="Wallis and Futuna Is">Wallis and Futuna Is</option><option value="Western Sahara">Western Sahara</option><option value="Yemen">Yemen</option><option value="Yugoslavia">Yugoslavia</option><option value="Zaire">Zaire</option><option value="Zambia">Zambia</option><option value="Zimbabwe">Zimbabwe</option>
						</select>
					</div>
					<div class="mlist-field mlist-field1">
						<label for="firstname">First name</label>
						<input id="firstname" name="firstname" type="text" />
					</div>
					<div class="mlist-field">
						<label for="birthday">Birth Date</label>
						<select name="dobMonth" id="dobMonth">
							<option value="">Month</option><option value="1">January</option><option value="2">February</option><option value="3">March</option><option value="4">April</option><option value="5">May</option><option value="6">June</option><option value="7">July</option><option value="8">August</option><option value="9">September</option><option value="10">October</option><option value="11">November</option><option value="12">December</option>
						</select>
						<select name="dobDay" id="dobDay">
							<option value="">Day</option><option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option><option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8">8</option><option value="9">9</option><option value="10">10</option><option value="11">11</option><option value="12">12</option><option value="13">13</option><option value="14">14</option><option value="15">15</option><option value="16">16</option><option value="17">17</option><option value="18">18</option><option value="19">19</option><option value="20">20</option><option value="21">21</option><option value="22">22</option><option value="23">23</option><option value="24">24</option><option value="25">25</option><option value="26">26</option><option value="27">27</option><option value="28">28</option><option value="29">29</option><option value="30">30</option><option value="31">31</option>
						</select>
					</div>
					<input id="mainListId" name="newsletterId" type="hidden" value="14073863"/>
					<input id="Datasource" name="Datasource" type="hidden" value="[Artist Name]_website_newsletter_optin" />
					<div class="mlist-checks">
						<input type="checkbox" class="global-list-checkbox" name="newsletterId" value="[field_label_list_id]"/>
						<label class="global-list" for="global-list">[site_variables:mlist_global_optin_language]</label>
					</div>
				</div>
				<div class="mlist-submit-new">
					<input class="submit" type="submit" value="Submit"/>
				</div>
				<div class="termsWrapper second">
					<div class="terms">
						<a href="javascript:void(0)" class="terms"></a>
					</div>
					<div class="termsContent">
						[site_variables:terms]
					</div>
				</div>
			</div>
		</div><!-- //Thank you block -->
		<!--terms -->
	</div>
</div>';
  $handler->display->display_options['fields']['field_newsletter_id']['element_label_colon'] = FALSE;
  /* Field: Content: Label List Id */
  $handler->display->display_options['fields']['field_label_list_id']['id'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['table'] = 'field_data_field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['field'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['label'] = '';
  $handler->display->display_options['fields']['field_label_list_id']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rates']['id'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['table'] = 'field_data_field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['field'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['label'] = '';
  $handler->display->display_options['fields']['field_display_rates']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_display_rates']['type'] = 'list_key';
  /* Field: Content: Preview mode */
  $handler->display->display_options['fields']['field_preview_mode']['id'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['table'] = 'field_data_field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['field'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['label'] = '';
  $handler->display->display_options['fields']['field_preview_mode']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox Height */
  $handler->display->display_options['fields']['field_lbox_height']['id'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['table'] = 'field_data_field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['field'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_height']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox Width */
  $handler->display->display_options['fields']['field_lbox_width']['id'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['table'] = 'field_data_field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['field'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_width']['element_label_colon'] = FALSE;
  /* Field: Content: Timeout */
  $handler->display->display_options['fields']['field_toaster_timeout']['id'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['table'] = 'field_data_field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['field'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_timeout']['element_label_colon'] = FALSE;
  /* Field: Content: Toaster Position */
  $handler->display->display_options['fields']['field_toaster_position']['id'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['table'] = 'field_data_field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['field'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_position']['element_label_colon'] = FALSE;
  /* Field: Content: Distance from Position */
  $handler->display->display_options['fields']['field_turnkey_height']['id'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['table'] = 'field_data_field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['field'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['label'] = '';
  $handler->display->display_options['fields']['field_turnkey_height']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_turnkey_height']['settings'] = array(
    'thousand_separator' => '',
    'prefix_suffix' => 1,
  );
  $handler->display->display_options['defaults']['sorts'] = FALSE;
  /* Sort criterion: Content: Updated date */
  $handler->display->display_options['sorts']['changed']['id'] = 'changed';
  $handler->display->display_options['sorts']['changed']['table'] = 'node';
  $handler->display->display_options['sorts']['changed']['field'] = 'changed';
  $handler->display->display_options['sorts']['changed']['order'] = 'DESC';
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
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
    'turnkey_toaster' => 'turnkey_toaster',
  );
  $handler->display->display_options['block_description'] = 'Turnkey-Toaster-JSON';
  $translatables['turnkey_lightbox_toaster'] = array(
    t('Master'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Turnkey Lightbox JSON'),
    t('<div class="mlist-outer-wrapper embedded-inline-list country-detect footer lightbox">
	<div class="mlist-wrapper">
		<!-- Ajax Loader Icon -->
		<div id="ajaxLoader" style="display: none">
		</div><!-- //ajaxLoader-->
		<form class="join-button-form" id="signupForm2" method="get" action="">
			<fieldset>
				<div class="mlist-header">
					Be one of the first to know about our tour dates, video premieres, and special announcements.
				</div>
				<div id="serverError" style="display: none">
					Unable to process your request at this time. Please try again later.
				</div>
				<div id="errorMsg" style="display: none">
					Please fill out the correct information.
				</div>
				<div class="mlist-field">
					<input id="email" name="email" type="email" class="m-required m-email" placeholder="Email Address"/>
				</div>
				<div class="mlist-submit">
					<input class="submit" type="submit" value="Sign Up"/>
				</div>
				<div class="primary-list-values">
					<input id="autoreply" name="autoreply" type="hidden" value="yes" />
					<input id="signuptype" name="signuptype" type="hidden" value="any" />
					<input id="Datasource" name="Datasource" type="hidden" value="[field_data_source]" />
					<input id="mainListId" name="newsletterId" type="hidden" value="[field_newsletter_id]"/>
				</div>
			</fieldset>
			<div class="termsWrapper first">
				<div class="terms">
					<a href="javascript:void(0)" class="terms"></a>
				</div>
				<div class="termsContent">
					[site_variables:terms]
				</div>
			</div>
		</form>
		<div id="thankyouBlock" style="display: none">
			<div class="thankyou-message embed" style="font-weight:bold;">
				Thank you!
			</div>
			<div id="thankyoublock-innerwrapper">
				<div class="secondFormCloseWrap">
					<a href="javascript:void(0)">x</a>
				</div>
				<div class="thankswelcome-msg" style="text-align:left;font-size:15px;">
					[site_variables:mlist_label_subscription_callout]
				</div>
				<div id ="secondary-list-values" class="secondary-list-values">
					<div id="errorLabelContainer2" style="display: none">
						Please fill out the correct information.
					</div>
					<div class="mlist-field postalcode">
						<label for="postalcode">Postal Code</label>
						<input id="postalcode" name="postalcode" type="text" autofocus/>
					</div>
					<div class="mlist-field country">
						<label for="country"> Country </label>
						<select id="country" name="country" class="m-required">
							<option value="">Select a country</option>
							<option value="United States">United States</option>
							<option value="Afghanistan">Afghanistan</option>
							<option value="Albania">Albania</option>
							<option value="Algeria">Algeria</option>
							<option value="American Samoa">American Samoa</option>
							<option value="Andorra">Andorra</option>
							<option value="Angola">Angola</option>
							<option value="Anguilla">Anguilla</option>
							<option value="Antarctica">Antarctica</option>
							<option value="Antigua and Barbuda">Antigua and Barbuda</option>
							<option value="Argentina">Argentina</option>
							<option value="Armenia">Armenia</option>
							<option value="Aruba">Aruba</option>
							<option value="Australia">Australia</option>
							<option value="Austria">Austria</option>
							<option value="Azerbaijan">Azerbaijan</option>
							<option value="Bahamas">Bahamas</option>
							<option value="Bahrain">Bahrain</option>
							<option value="Bangladesh">Bangladesh</option>
							<option value="Barbados">Barbados</option>
							<option value="Belarus">Belarus</option>
							<option value="Belgium">Belgium</option>
							<option value="Belize">Belize</option>
							<option value="Benin">Benin</option>
							<option value="Bermuda">Bermuda</option>
							<option value="Bhutan">Bhutan</option>
							<option value="Bolivia">Bolivia</option>
							<option value="Bosnia &amp; Herzegovina">Bosnia &amp; Herzegovina</option>
							<option value="Botswana">Botswana</option>
							<option value="Bouvet Island">Bouvet Island</option>
							<option value="Brazil">Brazil</option>
							<option value="British Ind Ocean Ter ">British Ind Ocean Ter </option>
							<option value="Brunei Darussalam">Brunei Darussalam</option>
							<option value="Bulgaria">Bulgaria</option>
							<option value="Burkina Faso">Burkina Faso</option>
							<option value="Burundi">Burundi</option>
							<option value="Cambodia">Cambodia</option>
							<option value="Cameroon">Cameroon</option>
							<option value="Canada">Canada</option>
							<option value="Cape Verde">Cape Verde</option>
							<option value="Cayman Islands">Cayman Islands</option>
							<option value="Central African Rep">Central African Rep</option>
							<option value="Chad">Chad</option>
							<option value="Chile">Chile</option>
							<option value="China">China</option>
							<option value="Christmas Island">Christmas Island</option>
							<option value="Cocos (Keeling Is)">Cocos (Keeling Is)</option>
							<option value="Colombia">Colombia</option>
							<option value="Comoros">Comoros</option>
							<option value="Congo">Congo</option>
							<option value="Cook Islands">Cook Islands</option>
							<option value="Costa Rica">Costa Rica</option>
							<option value="Cote D\'ivoire">Cote D\'Ivoire </option>
							<option value="Croatia (Hrvatska)">Croatia (Hrvatska)</option>
							<option value="Cuba">Cuba</option>
							<option value="Cyprus">Cyprus</option>
							<option value="Czech Republic">Czech Republic</option>
							<option value="Denmark">Denmark</option>
							<option value="Djibouti">Djibouti</option>
							<option value="Dominica">Dominica</option>
							<option value="Dominican Republic">Dominican Republic</option>
							<option value="East Timor">East Timor</option>
							<option value="Ecuador">Ecuador</option>
							<option value="Egypt">Egypt</option>
							<option value="El Salvador">El Salvador</option>
							<option value="Equatorial Guinea">Equatorial Guinea</option>
							<option value="Eritrea">Eritrea</option>
							<option value="Estonia">Estonia</option>
							<option value="Ethiopia">Ethiopia</option>
							<option value="Falkland Islands ">Falkland Islands </option>
							<option value="Faroe Islands">Faroe Islands</option>
							<option value="Fiji">Fiji</option>
							<option value="Finland">Finland</option>
							<option value="France">France</option><option value="France, Metro">France, Metro</option><option value="French Guiana">French Guiana</option><option value="French Polynesia">French Polynesia</option><option value="French Southern Ter">French Southern Ter</option><option value="Gabon">Gabon</option><option value="Gambia">Gambia</option><option value="Georgia">Georgia</option><option value="Georgia and S. Sand Is">Georgia and S. Sand Is</option><option value="Germany">Germany</option><option value="Ghana">Ghana</option><option value="Gibraltar">Gibraltar</option><option value="Greece">Greece</option><option value="Greenland">Greenland</option><option value="Grenada">Grenada</option><option value="Guadeloupe">Guadeloupe</option><option value="Guam">Guam</option><option value="Guatemala">Guatemala</option><option value="Guinea">Guinea</option><option value="Guinea-Bissau">Guinea-Bissau</option><option value="Guyana">Guyana</option><option value="Haiti">Haiti</option><option value="Heard &amp; McDonald Is">Heard &amp; McDonald Is</option><option value="Honduras">Honduras</option><option value="Hong Kong">Hong Kong</option><option value="Hungary">Hungary</option><option value="Iceland">Iceland</option><option value="India">India</option><option value="Indonesia">Indonesia</option><option value="Iran">Iran</option><option value="Iraq">Iraq</option><option value="Ireland">Ireland</option><option value="Israel">Israel</option><option value="Italy">Italy</option><option value="Jamaica">Jamaica</option><option value="Japan">Japan</option><option value="Jordan">Jordan</option><option value="Kazakhstan">Kazakhstan</option><option value="Kenya">Kenya</option><option value="Kiribati">Kiribati</option><option value="Korea (North) ">Korea (North) </option><option value="Korea (South)">Korea (South)</option><option value="Kuwait">Kuwait</option><option value="Kyrgyzstan ">Kyrgyzstan </option><option value="Laos">Laos</option><option value="Latvia">Latvia</option><option value="Lebanon">Lebanon</option><option value="Lesotho">Lesotho</option><option value="Liberia">Liberia</option><option value="Libya">Libya</option><option value="Liechtenstein">Liechtenstein</option><option value="Lithuania">Lithuania</option><option value="Luxembourg">Luxembourg</option><option value="Macau">Macau</option><option value="Macedonia">Macedonia</option><option value="Madagascar">Madagascar</option><option value="Malawi">Malawi</option><option value="Malaysia">Malaysia</option><option value="Maldives">Maldives</option><option value="Mali">Mali</option><option value="Malta">Malta</option><option value="Marshall Islands">Marshall Islands</option><option value="Martinique">Martinique</option><option value="Mauritania">Mauritania</option><option value="Mauritius">Mauritius</option><option value="Mayotte">Mayotte</option><option value="Mexico">Mexico</option><option value="Micronesia">Micronesia</option><option value="Moldova">Moldova</option><option value="Monaco">Monaco</option><option value="Mongolia">Mongolia</option><option value="Montserrat">Montserrat</option><option value="Morocco">Morocco</option><option value="Mozambique">Mozambique</option><option value="Myanmar">Myanmar</option><option value="Namibia">Namibia</option><option value="Nauru">Nauru</option><option value="Nepal">Nepal</option><option value="Netherlands">Netherlands</option><option value="Netherlands Antilles">Netherlands Antilles</option><option value="Neutral Zone Saudi/Iraq">Neutral Zone Saudi/Iraq</option><option value="New Caledonia">New Caledonia</option><option value="New Zealand">New Zealand</option><option value="Nicaragua">Nicaragua</option><option value="Niger">Niger</option><option value="Nigeria">Nigeria</option><option value="Niue">Niue</option><option value="Norfolk Island">Norfolk Island</option><option value="Northern Mariana Is">Northern Mariana Is</option><option value="Norway">Norway</option><option value="Oman">Oman</option><option value="Pakistan">Pakistan</option><option value="Palau">Palau</option><option value="Panama">Panama</option><option value="Papua New Guinea">Papua New Guinea</option><option value="Paraguay">Paraguay</option><option value="Peru">Peru</option><option value="Philippines">Philippines</option><option value="Pitcairn">Pitcairn</option><option value="Poland">Poland</option><option value="Portugal">Portugal</option><option value="Puerto Rico">Puerto Rico</option><option value="Qatar">Qatar</option><option value="Reunion">Reunion</option><option value="Romania">Romania</option><option value="Russian Federation">Russian Federation</option><option value="Rwanda">Rwanda</option><option value="Saint Kitts and Nevis">Saint Kitts and Nevis</option><option value="Saint Lucia">Saint Lucia</option><option value="St. Vincent/Grenadines ">St. Vincent/Grenadines </option><option value="Samoa">Samoa</option><option value="San Marino">San Marino</option><option value="Sao Tome and Principe">Sao Tome and Principe</option><option value="Saudi Arabia">Saudi Arabia</option><option value="Senegal">Senegal</option><option value="Seychelles">Seychelles</option><option value="Sierra Leone">Sierra Leone</option><option value="Singapore">Singapore</option><option value="Slovakia (Slovak Rep)">Slovakia (Slovak Rep)</option><option value="Slovenia">Slovenia</option><option value="Solomon Islands">Solomon Islands</option><option value="Somalia">Somalia</option><option value="South Africa">South Africa</option><option value="Soviet Union (former)">Soviet Union (former)</option><option value="Spain">Spain</option><option value="Sri Lanka">Sri Lanka</option><option value="St. Helena">St. Helena</option><option value="St. Pierre and Miquelo ">St. Pierre and Miquelo </option><option value="Sudan">Sudan</option><option value="Suriname">Suriname</option><option value="Svalbard &amp; Jan Mayen">Svalbard &amp; Jan Mayen</option><option value="Swaziland">Swaziland</option><option value="Sweden">Sweden</option><option value="Switzerland">Switzerland</option><option value="Syria">Syria</option><option value="Taiwan">Taiwan</option><option value="Tajikistan">Tajikistan</option><option value="Tanzania">Tanzania</option><option value="Thailand">Thailand</option><option value="Togo">Togo</option><option value="Tokelau">Tokelau</option><option value="Tonga">Tonga</option><option value="Trinidad and Tobago">Trinidad and Tobago</option><option value="Tunisia">Tunisia</option><option value="Turkey">Turkey</option><option value="Turkmenistan">Turkmenistan</option><option value="Turks and Caicos Is">Turks and Caicos Is</option><option value="Tuvalu">Tuvalu</option><option value="US Minor Outlying Is">US Minor Outlying Is</option><option value="Uganda">Uganda</option><option value="Ukraine">Ukraine</option><option value="United Arab Emirates">United Arab Emirates</option><option value="United Kingdom">United Kingdom </option><option value="Uruguay">Uruguay</option><option value="Uzbekistan">Uzbekistan</option><option value="Vanuatu">Vanuatu</option><option value="Vatican City State ">Vatican City State </option><option value="Venezuela">Venezuela</option><option value="Viet Nam">Viet Nam</option><option value="Virgin Islands (Brit)">Virgin Islands (Brit)</option><option value="Virgin Islands (US)">Virgin Islands (US)</option><option value="Wallis and Futuna Is">Wallis and Futuna Is</option><option value="Western Sahara">Western Sahara</option><option value="Yemen">Yemen</option><option value="Yugoslavia">Yugoslavia</option><option value="Zaire">Zaire</option><option value="Zambia">Zambia</option><option value="Zimbabwe">Zimbabwe</option>
						</select>
					</div>
					<div class="mlist-field mlist-field1">
						<label for="firstname">First name</label>
						<input id="firstname" name="firstname" type="text" />
					</div>
					<div class="mlist-field">
						<label for="birthday">Birth Date</label>
						<select name="dobMonth" id="dobMonth">
							<option value="">Month</option><option value="1">January</option><option value="2">February</option><option value="3">March</option><option value="4">April</option><option value="5">May</option><option value="6">June</option><option value="7">July</option><option value="8">August</option><option value="9">September</option><option value="10">October</option><option value="11">November</option><option value="12">December</option>
						</select>
						<select name="dobDay" id="dobDay">
							<option value="">Day</option><option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option><option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8">8</option><option value="9">9</option><option value="10">10</option><option value="11">11</option><option value="12">12</option><option value="13">13</option><option value="14">14</option><option value="15">15</option><option value="16">16</option><option value="17">17</option><option value="18">18</option><option value="19">19</option><option value="20">20</option><option value="21">21</option><option value="22">22</option><option value="23">23</option><option value="24">24</option><option value="25">25</option><option value="26">26</option><option value="27">27</option><option value="28">28</option><option value="29">29</option><option value="30">30</option><option value="31">31</option>
						</select>
					</div>
					<input id="mainListId" name="newsletterId" type="hidden" value="14073863"/>
					<input id="Datasource" name="Datasource" type="hidden" value="[Artist Name]_website_newsletter_optin" />
					<div class="mlist-checks">
						<input type="checkbox" class="global-list-checkbox" name="newsletterId" value="[field_label_list_id]"/>
						<label class="global-list" for="global-list">[site_variables:mlist_global_optin_language]</label>
					</div>
				</div>
				<div class="mlist-submit-new">
					<input class="submit" type="submit" value="Submit"/>
				</div>
				<div class="termsWrapper second">
					<div class="terms">
						<a href="javascript:void(0)" class="terms"></a>
					</div>
					<div class="termsContent">
						[site_variables:terms]
					</div>
				</div>
			</div>
		</div><!-- //Thank you block -->
		<!--terms -->
	</div>
</div>'),
    t('<div class="merchWrapper">
  <div class="merchName">[field_merch_item_name]</div>
  <div class="merchDesc">[body]</div>
  <div class="merchPrice">[field_merch_item_price]</div>
  <div class="merchLink"><a href="[field_merch_item_link]">Buy</a></div>
</div>'),
    t('"[field_cta_button-title]":"[field_cta_button-url]"'),
    t('{[field_cta_button]}'),
    t('Turnkey-Lightbox-JSON'),
    t('Turnkey Toaster JSON'),
    t('JSON values for the Toaster content'),
    t('Turnkey-Toaster-JSON'),
  );
  $export['turnkey_lightbox_toaster'] = $view;

  return $export;
}
