Feature: Javascript libraries

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And I have javascript libraries enabled

  Scenario Outline: As a user, I can enable an '<library type>' JS library
    When I configure javascript libraries
    And I choose to add a new '<library type>' javascript library
    And I enable the library
    Then I can use the javascript code on my site
    Examples:
    |library type|
    | upload     |
    | web url    |

  Scenario Outline: As a user, I can disable an '<library type>' JS library
    Given I have a '<library type>' javascript library enabled
    And I chose to disable it
    Then the javascript code is no longer available on my site
    Examples:
    |library type|
    | upload     |
    | web url    |

  Scenario: As a user, I can cache JS libraries for performance
    Given I have some javascript libraries enabled
    And I chose to chache my js libraries
    Then my javascript code is still available
    And I clear all the caches on my site
    Then my javascript code is still available
    And I disable javascript library caching
    And I clear all the caches on my site
    Then my javascript code is still available

  Scenario Outline: As a user, I can use the included JS libraries
    When I '<enable>' and included JS library
    Then the javascript code is '<availability>' availble for use on my site
    Examples:
    | enable  | availability |
    | enable  |              |
    | disable | not          |

  Scenario: As a user, I can use jCarousel to create a carousel view
    When I follow the steps in http://acquia.com/blog/creating-carousel-drupal-gardens
    Then I get a nice carousel on my site

