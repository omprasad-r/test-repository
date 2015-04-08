Feature: Webforms

  Background:
    Given a fresh gardens installation
    And the site has webform enabled
    And I am logged in as our testuser

  @smoke @utest
  Scenario: As a user, I can create a basic webform
    When I start to create a webform
    And I fill in "Title" with "zomgbbq"
    And I add a Text field to the webform
    And I add Radio buttons to the webform
    And I add a Drop-down List to the webform
    And I press "Publish"
    Then I should see a webform with a Text field
    And I should see a webform with Radio buttons
    And I should see a webform with a Drop-down List

  @smoke @utest
  Scenario: As a user, I can change the label of webform components
    When I start to create a webform
    And I fill in "Title" with "zomgbbq"
    And I add a Text field to the webform
    And I change the component's label to "How you doin?"
    And I add a File upload field to the webform
    And I change the component's label to "Sup bro?"
    And I add an email field to the webform
    And I change the component's label to "mailderp"
    And I publish the webform
    Then I should see a webform with a Text field
    And I should see a webform text field with the text "How you doin?"
    And I should see a webform file upload field with the text "Sup bro?"
    And I should see a webform email field with the text "mailderp"

  @smoke @utest
  Scenario: As a user, I can change every setting of the textfield component
    When I start to create a webform
    And I fill in "Title" with "ExtremeFeatureTest"
    And I add a Text field to the webform
    And I change the component's label to "inline is so 90s"
    And I change the component's label alignment to "Inline"
    And I change the component's default value to "defdefdef"
    And I change the component's prefix to "preprepre"
    And I change the component's suffix to "sufsufsuf"
    And I mark the component as required
    And I publish the webform
    Then I should see a webform with a Text field
    And I should see a webform textfield with the text "inline is so 90s"
    And I should see a webform textfield with a default value of "defdefdef"
    And I should see a webform textfield with a prefix of "preprepre"
    And I should see a webform textfield with a suffix of "sufsufsuf"
    And I should see a webform textfield which is marked as required
    And I should see a webform textfield with an inline label alignment


  @smoke @utest
  Scenario: As a user, I can change every setting of the radio button component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the check box component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the select list component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the email field component
    Given this hasn't been automated yet

  @smoke @utest

  @smoke @utest
  Scenario: As a user, I can change every setting of the file upload field component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the page break component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the HTML markup component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the fieldset component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change every setting of the hidden field component
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can delete a webform
    When I start to create a webform
    And I fill in "Title" with "zomgbbq"
    And I add a Text field to the webform
    And I publish the webform
    And I delete the current webform
    Then the content should be gone


  @smoke @utest
  Scenario: As a user, I can delete a webform from find content overlay
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can delete a webform from find content overlay using a checkbox
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can delete webform components
    When I start to create a webform
    And I fill in "Title" with "partialdeletion"
    And I add a Text field to the webform
    And I add a Text field to the webform
    And I add a Drop-down List to the webform
    And I add a Text field to the webform
    And I publish the webform
    And I start to edit the current webform
    And I remove all Text fields from the webform
    And I press "Save"
    Then I shouldn't see a webform with a Text field

  @smoke @utest
  Scenario: As a user, I can set a custom confirmation message
    When I start to create a webform
    And I fill in "Title" with "confirmation message test"
    And I add a Text field to the webform
    And I set the custom webform confirmation to "Well, well, what do we have here?"
    And I publish the webform
    And I start filling out the created webform
    And I fill in 'Text field' with 'derp'
    And I press 'Submit'
    Then I should see "Well, well, what do we have here?"

  @smoke @utest
  Scenario: As a user, I can create a webform as a block
    When I start to create a webform
    And I fill in "Title" with "nanananananamisssaigon"
    And I add a Text field to the webform
    And I enable the block creation for this webform
    And I publish the webform
    Then I should see a block with the name "Webform: nanananananamisssaigon" in the block list
#Selenium because of the overlay for submissions

  @smoke @utest @selenium
  Scenario: Anonymous and registered users can submit a webform and we'll end up with correct results
    When I start to create a webform
    And I fill in "Title" with "submissiontest"
    And I add a Text field to the webform
    And I publish the webform
    And I start filling out the created webform
    And I fill in 'Text field' with 'herp'
    And I press 'Submit'
    And I log out
    And I start filling out the created webform
    And I fill in 'Text field' with 'derp'
    And I press 'Submit'
    Given I am logged in as our testuser
    Then I should see 2 submissions on the results page of the current webform

  @smoke @utest
  Scenario: As a user, I can mark certain fields as required
    When I start to create a webform
    And I fill in "Title" with "submissiontest"
    And I add a Drop-down List to the webform
    And I add a Text field to the webform
    And I mark the recently added component as required
    And I publish the webform
    And I start filling out the created webform
    And I press 'Submit'
    Then I should see an error message with the text "is required"

  @smoke @utest
  Scenario: As a user, I can add an option to an already created webform
    When I start to create a webform
    And I fill in "Title" with "edittest"
    And I add a Drop-down List to the webform
    And I press "Publish"
    And I start to edit the current webform
    And I add the option "barfcake" to the drop-down list
    And I press "Save"
    Then I should see a webform drop-down list with the option "barfcake"

  @smoke @utest
  Scenario: As a user, I can limit webform submissions to registered users
    When I start to create a webform
    And I fill in "Title" with "nonanontest"
    And I add a Text field to the webform
    And I disable webform submissions for anonymous users
    And I publish the webform
    And I log out
    And I visit the current webform
    Then I should see a status message with the text "You must login or register to view this form"

  @smoke @utest
  Scenario: As a user, I can add new webforms to certain content types
    Given this hasn't been automated yet

  @smoke @utest
  Scenario Outline: As a user, I can enable and disable webforms for certain content types
    When I enable webforms for the <content_type> content type
    And I start to create a <content_type>
    Then I should see the webform builder
    When I disable webforms for the <content_type> content type
    Then I should not see the webform builder
  Examples:
    | content_type  |
    | article       |
    | page          |
    | poll          |
    | media-gallery |
    | blog          |
    | forum         |
    | webform       |

  @smoke @utest
  Scenario Outline: As a user, I can disable certain components on a webform
    When I disable the <component> component for webforms
    And I disable webforms for the blog content type
    And I start to create a blog
    Then I should not see a webform <component> component in the "Add field" tab
    When I enable the <component> component for webforms
    And I enable webforms for the blog content type
    And I start to create a blog
    Then I should see a webform <component> component in the "Add field" tab
  Examples:
    | component |
    | email     |
    | file      |

  @smoke @utest
  Scenario: A user can create a new webform from the webforms settings page
    When I go to the webform content type configuration page
    Then there should be a link to /node/add/webform

#Marc: There doesn't seem to be a cookie. At least I can't find one
  @smoke @utest @wip
  Scenario: A user can enable cookies for webforms
    When I enable cookies for webform submission tracking
    And I start to create a webform
    And I fill in "Title" with "cookietest"
    And I add a Text field to the webform
    And I publish the webform
    And I start filling out the created webform
    And I fill in 'Text field' with 'herp'
    And I press 'Submit'
    Then there should be a webform cookie

  @smoke @utest
  Scenario: As a user, I can change the webform submission settings
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to limit form submissions to a certain number
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to send a confirmation email after form submit
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to limit form submission access to certain users
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to display the complete form in the teaser
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to display a link to previous submissions
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the webform settings to show a custom submit button text
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the format for downloading submission results to "delimited text"
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user, I can change the format for downloading submission results to "Microsoft Excel"
    Given this hasn't been automated yet

