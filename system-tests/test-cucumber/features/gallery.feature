Feature: Gallery

Background:
  Given a fresh gardens installation
  Given I am logged in as our testuser

  Scenario: As a user, I can enable the media gallery module
    Given this hasn't been automated yet

  Scenario: As a user, I should see a 'no images selected' message when creating an empty gallery
    Given this hasn't been automated yet

  Scenario: As a user, I can add images in different formats to the gallery
    Given this hasn't been automated yet

  Scenario: As a user, I can remove an image from the gallery while in full page detail view
    Given this hasn't been automated yet

  Scenario: As a user, I can cancel the image removal action in the gallery
    Given this hasn't been automated yet

  Scenario: As a user, I can download an image from the gallery
    Given this hasn't been automated yet

  Scenario: As a user, I can display gallery images in a slide show
    Given this hasn't been automated yet

  Scenario: As a user, I can enable the most recent gallery block
    Given this hasn't been automated yet

  Scenario: As a user, I can disable the most recent gallery block
    Given this hasn't been automated yet

  Scenario: As a user, I can change the presentation settings of a gallery
    Given this hasn't been automated yet

  Scenario: As a user, I should see an error message when adding the same video media twice
    Given this hasn't been automated yet

  @smoke @utest
  Scenario: As a user I want to be able to create a new gallery
    Given I create a media gallery with the title 'beefcake' and the description 'barfcake'
    Then I should see content with the title 'beefcake'
    Then I should see content with 'barfcake' in the body

  @smoke @utest
  Scenario: As a user I want to be able to create multiple empty galleries
    Given I create a media gallery with the title 'beefcake' and the description 'barfcake'
    Then I should see content with the title 'beefcake'
    Then I should see content with 'barfcake' in the body
    Given I create a media gallery with the title 'numero dos' and the description 'olala'
    Then I should see content with the title 'numero dos'
    Then I should see content with 'olala' in the body

  #This crashes on capybara-webkit. Isn't particularily stable on selenium either, but meh...
  @selenium @utest
  Scenario: As a user I want to create multiple images to a gallery
    Given I create a media gallery with the title 'shoop' and the description 'da whoop'
    And I add 4 random images to the gallery
    Then I should see 4 images in the gallery
    Given I create a media gallery with the title 'barf' and the description 'arf'
    And I add 2 random images to the gallery
    Then I should see 2 images in the gallery

  #Probably some caching problem with our testing VMs
  #Gardens uses varnish and not the internal drupal cache
  @wip @utest
  Scenario: As a user I want to be able to delete a gallery
    Given I create a media gallery with the title 'shoop' and the description 'da whoop'
    Then the content should be available
    When I delete the current gallery
    Then the content should be gone

  #This crashes on capybara-webkit. Isn't particularily stable on selenium either, but meh...
  @selenium @utest
  Scenario: As a user I want to add images to a gallery
    Given I create a media gallery with the title 'shoop' and the description 'da whoop'
    And I add 4 random images to the gallery
    Then I should see 4 images in the gallery

  #This crashes on capybara-webkit. Isn't particularily stable on selenium either, but meh...
  @selenium @utest
  Scenario: As a user I want to add videos to a gallery
    Given I create a media gallery with the title 'shoop' and the description 'da whoop'
    And I add 2 random videos to the gallery
    Then I should see 2 videos in the gallery


  #This crashes on capybara-webkit. Isn't particularily stable on selenium either, but meh...
  @selenium @utest
  Scenario: As a user I want to change settings for the all galleries page
    Given I am going to the galleries page
    And I change the title for the all galleries page to "derp"
    Then I should see the page title "derp"

  @selenium @utest
  Scenario: As a user I want to be able to edit gallery settings
    Given I create a media gallery with the title 'hurr' and the description 'durr'
    And I add 2 random images to the gallery
    And I switch the gallery to display media in full screen
    And I click on a random image in the gallery
    Then I should see a fullscreen image
    And I switch the gallery to display media in a lightbox
    And I click on a random image in the gallery
    Then I should see a lightbox image

  @selenium @utest
  Scenario: As a user I want to be able to edit a gallery title
    Given I create a media gallery with the title 'hurr' and the description 'durr'
    And I edit the gallery
    And I change the title text to 'iChanged'
    Then I should see the page title 'iChanged'

  @selenium @utest
  Scenario: As a user I want to be able to edit a gallery description
    Given I change the window size to 1600x1200
    And I create a media gallery with the title 'hurr' and the description 'durr'
    And I edit the gallery
    And I change the description text to 'Friends, Romans, Countrymen'
    Then I should see the text 'Friends, Romans, Countrymen'

  @selenium @utest
  Scenario: As a user I want to be able to edit the license setting of an image
    Given I create a media gallery with the title 'hurr' and the description 'durr'
    And I add 2 random images to the gallery
    And I change the license setting of all images to "Attribution, Share alike"
    And I click on a random image in the gallery
    Then I should see the "Attribution" license logo
    And I should see the "Share Alike" license logo

  @selenium @utest
  Scenario Outline: As a user I want to be able to edit gallery rows and columns
    Given I create a media gallery with the title 'hurr' and the description 'durr'
    And I add 6 random images to the gallery
    And I change the gallery layout to <layout>
    Then I should see a gallery with a <layout> layout
    Examples:
      | layout  |
      | 2x1     |
      | 3x2     |
      | 2x2     |

  @selenium @utest
  Scenario: As a user I want to be able to create a gallery block
    Given I create a media gallery with the title 'gullury' and the description 'bluuurk'
    And I add 2 random images to the gallery
    And I enable the block creation for this gallery
    Then I should see a block with the name "Recent gallery items: gullury" in the block list
    When I visit the current gallery
    And I disable the block creation for this gallery
    Then I should not see a block with the name "Recent gallery items: gullury" in the block list

  @selenium @utest
  Scenario: As a user I can delete all recently created galleries in the content admin interface
    Given I create a media gallery with the title 'gonesoonone' and the description 'bluuurk'
    And I add 2 random images to the gallery
    Given I create a media gallery with the title 'gonesoontwo' and the description 'blaaaag'
    And I add 2 random images to the gallery
    And I delete all Gallery content
    And I go to the galleries page
    Then I should see the text "No galleries have been set up yet"

  @selenium @smoke @utest
  Scenario: As a user I should see a warning when deleting media that is currently in use
    Given I create a media gallery with the title 'deleteinhere' and the description 'gnnnffff'
    And I add 1 random image to the gallery
    When I delete the most recent image using the content administration
    Then I should see the text "Deleting this file may cause unintended results."
    When I press "Delete"
    Then I should see the text "was deleted"

  @selenium @smoke
  Scenario: As a user I can delete media that isn't in use when using the media content administration
    Given I add 1 random image to the site
    And I delete and confirm the most recent image using the content administration
    Then I should see the text "was deleted"

  Scenario: As a user I can access the gallery settings using the configuration page
    Given I visit the configuration page
    Then there should be a link to /admin/config/media/galleries

  @selenium @utest
  Scenario: As a user I can drag and drop images within a gallery
    Given I create a media gallery with the title 'gullury' and the description 'bluuurk'
    And I change the window size to 1600x1200
    And I add 5 random images to the gallery
    And I should be able to drag images to the first position
    And I should be able to drag images to the last position

