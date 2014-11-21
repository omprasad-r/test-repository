Feature: As a site developer I want to work as long as possible on a site with no aggregation of JS and CSS

  Background:
    Given a fresh gardens installation

  Scenario: CSS/JS aggregation is automatically re-enabled
    When acsf_disable_automatic_aggregation is not set
    And I disable CSS/JS aggregation
    And cron runs
    Then I can see that CSS/JS aggregation is automatically re-enabled

  Scenario: CSS/JS aggregation is not automatically re-enabled
    When acsf_disable_automatic_aggregation is set to a truthy value
    And I disable CSS/JS aggregation
    And cron runs
    Then I can see that CSS/JS aggregation is not automatically re-enabled
