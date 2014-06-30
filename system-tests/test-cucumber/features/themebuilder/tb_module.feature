Feature: Basic themebuilder module behaviour

  Scenario: As a drupalgardens.com user, I cannot disable the themebuilder modules
    Given I am logged in as a user on drupalgardens.com (or gsteamer)
    And I have created a new site (under any free or paid subscription)
    And I visit my new site
    And I visit the modules page at admin/modules
    Then I cannot see any checkbox available for enabling or disabling the "Themebuilder"

  Scenario: As a Site Factory user (excluding drupalgardens.com), I can see and disable themebuilder modules
    Given I am logged into a Site Factory installation other than drupalgardens.com (or gsteamer)
    And I have created a new site
    And I visit my new site
    And I visit the modules page at admin/modules
    Then I can see a checkbox labelled "Themebuilder" which I am able to uncheck

  Scenario: As a Site Factory user (excluding drupalgardens.com), I see no other themebuilder modules than "Thembuilder" and "Themebuilder preview"
    Given I am logged into a Site Factory installation other than drupalgardens.com (or gsteamer)
    And I have created a new site
    And I visit my new site
    And I visit the modules page at admin/modules
    Then The only modules I can see containing the word "themebuilder" are "Themebuilder" and "Themebuilder preview"
