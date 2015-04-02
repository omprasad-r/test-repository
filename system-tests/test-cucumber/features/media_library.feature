Feature: Media library

  Background:
    Given a fresh gardens installation
    And media gallery is enabled
    And I am logged in as our testuser

  @utest
  Scenario: As a user, I can add multiple images to a media library
    When I select the media tab
    And I add files
    Then the added files are in media library

  @utest
  Scenario: As a user, I can see that the media library thumbnails are generated properly
    When I select the media tab
    And I select the thumbnail view
    Then all the thumbnails render correctly

  @utest
  Scenario: As a user, I can replace images in already created content
    Given I have content with a media item
    When I edit that content
    And I delete the media item
    And I add a new media item
    And I save the content
    Then I can see the new media in my content

  @utest
  Scenario Outline: As a user, I can add different file types to a media library
    When I add media of type "<media_type>" to the library
    Then I can use that media on my site
    Examples:
    | media_type |
    |  jpg       |
    | jpeg       |
    | gif        |
    | png        |
    | txt        |
    | doc        |
    | pdf        |
    | mp3        |

  @utest
  Scenario: As a user, I can delete files from a media library
    Given files in the media library
    When I select an item
    And I click delete
    And I confirm the delete
    Then the file is deleted

  @utest
  Scenario: As a user, I can add media from embed.ly
    When I select the media tab
    And I add media using embed from url
    Then the added media are in media library

