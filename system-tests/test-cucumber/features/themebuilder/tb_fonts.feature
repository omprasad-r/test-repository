Feature: Theme Builder fonts

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario Outline: As user, I can change the font color by using the color picker
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I pick item "<desired_color>" from color picker "style-font-color"
    Then I should see that "site name link" equals "style-font-color" color
    Examples:
      | desired_color |
      |     a         |
      |     b         |
      |     c         |
      |     d         |
      |     e         |

  @selenium
  Scenario Outline: As user, I can change the font size by using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I move the "style-font-size" slider to 20 percent
    Then I should see that "<desired_element>" equals "style-font-size" font size
    And I move the "style-font-size" slider to 35 percent
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element |
      | site name link  |
      | site slogan     |

  @poltergeist
  Scenario Outline: As user, I can change the font size by using the textfield
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I type "14" into "style-font-size"
    And I press the key "enter" on element "style-font-size"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    And I type "25" into "style-font-size"
    And I press the key "enter" on element "style-font-size"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element |
      | site name link  |
      | site slogan     |

      # Needs selenium because webkit does not report correct font family
  @selenium
  Scenario Outline: As user, I can change the font family
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I select "<desired_font_family>" from select box "style-font-family"
    Then I should see that "site name link" equals "style-font-family" font family
    Examples:
      | desired_font_family |
      |     Arial           |
      |     Courier         |
      |     Times           |
      |     Palatino        |
      |     Lucida Sans     |
      |     Bradley Hand    |
      |     Georgia         |
      |     Monaco          |

  Scenario Outline: As user, I can change the font weight, style, decoration and transform
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press "style-<desired_id>" in the "#themebuilder-font-editor" section
    Then I should see that "<desired_element>" equals "style-<desired_id>" <desired_property>
    Examples:
      | desired_element | desired_id        | desired_property  |
      | site name link  | font-weight       | font weight       |
      | site slogan     | font-weight       | font weight       |
      | site name link  | font-style        | font style        |
      | site slogan     | font-style        | font style        |
      | site name link  | text-decoration   | text decoration   |
      | site slogan     | text-decoration   | text decoration   |
      | site name link  | font-transform    | text transform    |
      | site slogan     | font-transform    | text transform    |

  Scenario Outline: As user, I can change the font algnment
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press element "#<desired_id>" in the "div.text-align-panel" section
    Then I should see that "<desired_element>" equals "<desired_id>" text align
    Examples:
      | desired_element | desired_id          |
      | site name link  | text-align-left     |
      | site slogan     | text-align-left     |
      | site name link  | text-align-right    |
      | site slogan     | text-align-right    |
      | site name link  | text-align-center   |
      | site slogan     | text-align-center   |
      | site name link  | text-align-justify  |
      | site slogan     | text-align-justify  |

  Scenario Outline: As user, I can change and reload the font color by using the color picker
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I pick item "<desired_color>" from color picker "style-font-color"
    And I save the current theme as "theme_<desired_color>"
    And I load the theme "theme_<desired_color>"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-font-color" color
    Examples:
      | desired_color |
      |     a         |
      |     b         |
      |     c         |
      |     d         |
      |     e         |

  @selenium
  Scenario Outline: As user, I can change and reload the font size by using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I move the "style-font-size" slider to <desired_percentage> percent
    And I save the current theme as "theme_<desired_percentage>"
    And I load the theme "theme_<desired_percentage>"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element | desired_percentage  |
      | site name link  |       20            |
      | site name link  |       45            |
      | site slogan     |       50            |
      | site slogan     |       65            |

  Scenario Outline: As user, I can change and reload the font size by using the textfield
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I type "14" into "style-font-size"
    And I save the current theme as "test_theme"
    And I load the theme "test_theme"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element |
      | site name link  |
      | site slogan     |

      # Needs selenium because webkit does not report correct font family
  @selenium
  Scenario Outline: As user, I can change and reload the font family
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I select "<desired_font_family>" from select box "style-font-family"
    And I save the current theme as "theme_<desired_font_family>"
    And I load the theme "theme_<desired_font_family>"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-font-family" font family
    Examples:
      | desired_font_family |
      |     Arial           |
      |     Courier         |
      |     Times           |
      |     Palatino        |
      |     Lucida Sans     |
      |     Bradley Hand    |
      |     Georgia         |
      |     Monaco          |

  Scenario Outline: As user, I can change and reload the font weight, style, decoration and transform
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press "style-<desired_id>" in the "#themebuilder-font-editor" section
    And I save the current theme as "theme_<desired_id>"
    And I load the theme "theme_<desired_id>"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-<desired_id>" <desired_property>
    Examples:
      | desired_element | desired_id        | desired_property  |
      | site name link  | font-weight       | font weight       |
      | site slogan     | font-weight       | font weight       |
      | site name link  | font-style        | font style        |
      | site slogan     | font-style        | font style        |
      | site name link  | text-decoration   | text decoration   |
      | site slogan     | text-decoration   | text decoration   |
      | site name link  | font-transform    | text transform    |
      | site slogan     | font-transform    | text transform    |

      # webkit is detecting wrong alignment
  @selenium
  Scenario Outline: As user, I can change and reload the font algnment
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press element "#<desired_id>" in the "div.text-align-panel" section
    And I save the current theme as "theme_<desired_id>"
    And I load the theme "theme_<desired_id>"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "<desired_id>" text align
    Examples:
      | desired_element | desired_id          |
      | site name link  | text-align-left     |
      | site slogan     | text-align-left     |
      | site name link  | text-align-right    |
      | site slogan     | text-align-right    |
      | site name link  | text-align-center   |
      | site slogan     | text-align-center   |
      | site name link  | text-align-justify  |
      | site slogan     | text-align-justify  |

  Scenario Outline: As user, I can change and publish the font color by using the color picker
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "kenwood"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I pick item "<desired_color>" from color picker "style-font-color"
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-font-color" color
    Examples:
      | desired_color |
      |     a         |
      |     b         |
      |     c         |
      |     d         |
      |     e         |


  @selenium
  Scenario Outline: As user, I can change and publish the font size by using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "kenwood"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I move the "style-font-size" slider to <desired_percentage> percent
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element | desired_percentage  |
      | site name link  |       20            |
      | site name link  |       45            |
      | site slogan     |       50            |
      | site slogan     |       65            |

  Scenario Outline: As user, I can change and publish the font size by using the textfield
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "kenwood"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I type "14" into "style-font-size"
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-font-size" font size
    Examples:
      | desired_element |
      | site name link  |
      | site slogan     |

      # Needs selenium because webkit does not report correct font family
  @selenium
  Scenario Outline: As user, I can change and publish the font family
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "kenwood"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I select "<desired_font_family>" from select box "style-font-family"
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-font-family" font family
    Examples:
      | desired_font_family |
      |     Arial           |
      |     Courier         |
      |     Times           |
      |     Palatino        |
      |     Lucida Sans     |
      |     Bradley Hand    |
      |     Georgia         |
      |     Monaco          |

  Scenario Outline: As user, I can change and publish the font weight, style, decoration and transform
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "broadway"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press "style-<desired_id>" in the "#themebuilder-font-editor" section
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "style-<desired_id>" <desired_property>
    Examples:
      | desired_element | desired_id        | desired_property  |
      | site name link  | font-weight       | font weight       |
      | site slogan     | font-weight       | font weight       |
      | site name link  | font-style        | font style        |
      | site slogan     | font-style        | font style        |
      | site name link  | text-decoration   | text decoration   |
      | site slogan     | text-decoration   | text decoration   |
      | site name link  | font-transform    | text transform    |
      | site slogan     | font-transform    | text transform    |

      # webkit is detecting wrong alignment
  @selenium
  Scenario Outline: As user, I can change and publish the font algnment
    Given I am on "the homepage"
    And I open the theme builder
    And I load the Gardens theme "broadway"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    And I press element "#<desired_id>" in the "div.text-align-panel" section
    And I publish the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<desired_element>"
    Then I should see that "<desired_element>" equals "<desired_id>" text align
    Examples:
      | desired_element | desired_id          |
      | site name link  | text-align-left     |
      | site slogan     | text-align-left     |
      | site name link  | text-align-right    |
      | site slogan     | text-align-right    |
      | site name link  | text-align-center   |
      | site slogan     | text-align-center   |
      | site name link  | text-align-justify  |
      | site slogan     | text-align-justify  |

  @selenium
  Scenario Outline: As a user, I can change the font face settings and preview, save and publish them
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I change font family to "Bradley Hand"
    And I remember the font family of "site name link" as "expected_font_family"
    And I change font weight to "bold"
    And I remember the font weight of "site name link" as "expected_font_weight"
    And I change font style to "italic"
    And I remember the font style of "site name link" as "expected_font_style"
    When I <desired_action> and reload the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I remember the font family of "site name link" as "actual_font_family"
    And I remember the font weight of "site name link" as "actual_font_weight"
    And I remember the font style of "site name link" as "actual_font_style"
    Then I should see that "site name link" equals "style-font-family" font family
    And I should see that "site name link" equals "style-font-weight" font weight
    And I should see that "site name link" equals "style-font-style" font style
    And I should see that remembered value "actual_font_family" equals "expected_font_family"
    And I should see that remembered value "actual_font_weight" equals "expected_font_weight"
    And I should see that remembered value "actual_font_style" equals "expected_font_style"
    Examples:
      | desired_action  |
      |   preview       |
      |   save          |
      |   save_as       |
      |   publish       |


  Scenario: As a user, I can change font attributes using different themes
    Given this hasn't been implemented yet

  Scenario: As a user, I can show and hide the power theming
    Given this hasn't been implemented yet

