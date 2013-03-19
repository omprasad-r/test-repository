// First: tell BrowserMob if you want a real web browser/RBU script
var selenium = browserMob.openBrowser();
// You an optionally set the simulated bandwidth for the script
// (max of 100KB/sec for VUs and 768KB/sec for RBUs)
// browserMob.setSimulatedBps(50 * 1024 * 8); // 50KB/sec

// You can also get the unique user number (0 -> max users)...
var userNum = browserMob.getUserNum();
// ... and transaction count for that specific user
var txCount = browserMob.getTxCount();

var tbOpen = false;

var Gardens = Gardens ||
{};

/**
 *
 * @param {Object} gardenerBase
 * @param {Object} user
 * @param {Object} password
 */
Gardens.login = function(gardensUrl, user, password){
    try {
        selenium.open(gardensUrl);
        if (selenium.isElementPresent("Link=(logout)")) {
            selenium.click("Link=(logout)");
        }
        selenium.waitForElementPresent("Link=Log in or register");
        selenium.click("Link=Log in or register");
        browserMob.pause(1000);
        browserMob.beginStop('selectOverlay');
        selenium.selectFrame("overlay-element");
        selenium.waitForElementPresent("edit-name");
        selenium.type("edit-name", user);
        selenium.type("edit-pass", password);
        browserMob.endStep();
        browserMob.beginStep('submitLogin');
        selenium.clickAndWait("edit-submit");
        browserMob.endStep();
    } 
    catch (loginException) {
        browserMob.log("OMG:" + loginException);
        throw loginException;
    }
    
};
var Themer = Themer ||
{};

Themer.exitThemeBuilder = function(){
    browserMob.log("Closing Themebuilder");
    if (tbOpen) {
        browserMob.log("Actually Trying to close");
        selenium.waitForCondition("!selenium.isVisible(\"themebuilder-veil\")", 30000);
        selenium.waitForElementPresent("themebuilder-exit-button");
        selenium.click("themebuilder-exit-button");
        browserMob.pause(10000);
    }
    else {
        browserMob.log("Test thinks that themebuilder is already closed");
    }
    tbOpen = false;
};

Themer.waitForThemeBuilder = function(){
    browserMob.log("Waiting for TB to complete opening");
    selenium.waitForElementPresent("themebuilder-exit-button");
    selenium.waitForElementPresent("link=Gardens*");
};

/**
 * Open TB, wait until the close TB element
 */
Themer.startThemeBuilder = function(forceStart){
    browserMob.log("Starting Themebuilder");
    var confMess; // potential confirmaition messages
    if (typeof forceStart === 'undefined') {
        forceStart = true;
    }
    else 
        if (!forceStart) {
            forceStart = false;
        }
        else {
            forceStart = true;
        }
    try {
        if (selenium.isConfirmationPresent() && forceStart) {
            browserMob.log("Trying to get a confirmation");
            confMess = selenium.getConfirmation();
            browserMob.log(confMess);
        }
        if (forceStart) {
            selenium.chooseCancelOnNextConfirmation();
        }
        else {
            selenium.chooseOkOnNextConfirmation();
        }
    } 
    catch (nonConfException) {
        browserMob.log("Thought there was a confirmation there was not: " + nonConfException);
        
    }
    selenium.setTimeout(120000);
    selenium.waitForElementPresent("toolbar-link-admin-appearance");
    browserMob.pause(1000);
    selenium.click("toolbar-link-admin-appearance");
    browserMob.log("Clicked on the appearance button");
    browserMob.pause(1000);
    try {
        if (selenium.isConfirmationPresent && forceStart) {
            browserMob.log("Trying to get a confirmation");
            confMess = selenium.getConfirmation();
            browserMob.log(confMess);
            browserMob.log("Trying to open TB again");
            selenium.click("toolbar-link-admin-appearance");
        }
    } 
    catch (tbNoOpenException) {
        browserMob.log("Thought there was a confirmation there was not: " + tbNoOpenException);
    }
    selenium.chooseOkOnNextConfirmation();
    Themer.waitForThemeBuilder();
    try {
        if (selenium.isConfirmationPresent && forceStart) {
            browserMob.log("Trying to get a confirmation");
            confMess = selenium.getConfirmation();
            browserMob.log(confMess);
        }
    } 
    catch (waitForTBException) {
        browserMob.log("Thought there was a confirmation there was not: " + waitForTBException);
    }
    selenium.chooseOkOnNextConfirmation();
    tbOpen = true;
};

/**
 * returns true if the status bar is bumped
 * blocks on the status bar bumping
 */
Themer.statusBarBumped = function statusBarBumped(){
    browserMob.log("Waiting for the status bar to bump");
    var bumped = false;
    try {
        selenium.waitForCondition("selenium.isVisible(\"themebuilder-status\")", 30000);
        selenium.waitForCondition("!selenium.isVisible(\"themebuilder-status\")", 30000);
        bumped = true;
    } 
    catch (fe) {
        browserMob.log("Oh there was an exception: " + fe);
        bumped = false;
    }
};

/**
 * saves theme with a specified name
 *
 * @param {String} themeName
 *   name of the theme
 *
 * @param {Boolean} overwrite
 *  overwrite an existing theme
 *
 */
Themer.saveThemeAs = function(themeName, overwrite){
    var confMess;
    if (typeof overwrite === 'undefined') {
        overwrite = true;
    }
    else 
        if (!overwrite) {
            overwrite = false;
        }
        else {
            overwrite = true;
        }
    if (overwrite) {
        selenium.chooseOkOnNextConfirmation();
        browserMob.log("Going to overwrite existing theme");
    }
    else {
        selenium.chooseCancelOnNextConfirmation();
    }
    try {
        selenium.waitForElementPresent("//div[@id='themebuilder-save']/button[contains(@class,'save') and not(@disabled)]");
        selenium.click("//div[@id='themebuilder-save']/button[contains(@class,'save') and not(@disabled)]");
        selenium.waitForElementPresent("edit-name");
        selenium.type("edit-name", themeName);
        selenium.waitForElementPresent("//button[@type='button']");
        selenium.click("//button[@type='button']");
        browserMob.pause(10000);
        try {
            if (selenium.isConfirmationPresent) {
                // get the confirmation and do nothing with it.
                browserMob.log("Trying to get confirmation");
                confMess = selenium.getConfirmation();
                browserMob.log(confMess);
            }
        } 
        catch (fe) {
            browserMob.log("saveThemeAs: did not seem to really have a confirmation: " + fe);
        }
        selenium.chooseOkOnNextConfirmation();
        if (overwrite) {
            Themer.statusBarBumped();
        }
    } 
    catch (saveAsException) {
        browserMob.log("Save As failed probably because there was an uncaught confirmation: " + saveAsException);
    }
};

/**
 * published a theme with a specified name if the
 * current theme has been published, just write it
 *
 * @param {String} themeName
 *   name of the theme
 *
 * @param {Boolean} overwrite
 *  overwrite an existing theme
 *
 */
Themer.publishTheme = function(themeName, overwrite){
    var confMess;
    if (typeof overwrite === 'undefined') {
        overwrite = true;
    }
    else 
        if (!overwrite) {
            overwrite = false;
        }
        else {
            overwrite = true;
        }
    if (overwrite) {
        selenium.chooseOkOnNextConfirmation();
    }
    else {
        selenium.chooseCancelOnNextConfirmation();
    }
    selenium.waitForElementPresent("//div[@id='themebuilder-save']/button[contains(@class,'publish')]");
    selenium.click("//div[@id='themebuilder-save']/button[contains(@class,'publish')]");
    if (!selenium.isElementPresent("css=div.themebuilder-loader")) {
        selenium.waitForElementPresent("edit-name-2");
        selenium.type("edit-name-2", themeName);
        selenium.waitForElementPresent("//button[@type='button']");
        selenium.click("//button[@type='button']");
        browserMob.pause(10000); //ned to be 10000 secodns becasue 1 second is not long enough as the system gets busy
        try {
            if (selenium.isConfirmationPresent && overwrite) {
                // get the confirmation and do nothing with it.
                confMess = selenium.getConfirmation();
                browserMob.log(confMess);
            }
            selenium.chooseOkOnNextConfirmation();
        } 
        catch (fe) {
            browserMob.log("Man o Man threw an exception trying to get a confirmation: " + fe);
        }
    }
    if (overwrite) {
        Themer.statusBarBumped();
    }
};

var loops = 10;
if (browserMob.isValidation()) {
    loops = 1; // no need to loop so many times if we're validating!
}

// BrowserMob groups transactions in to "steps". You can do work outside of a step, 
// but it won't be recorded in the reports and charts. To record timings, start a step.
//var gardenerBase = "http://site_" + userNum + "_test.gsteamer.acquia-sites.com";
var selenium = browserMob.openBrowser();

var gardenerBase = "http://qatest" + userNum + "site.gperf.acquia-sites.com/";
selenium.setTimeout(30000);
selenium.open(gardenerBase);
try {
  browserMob.beginTranaction();
  browserMob.beginStep("LoginToSite");
  Gardens.login(gardenerBase, "qatestuser", "qatestuser1");
  browserMob.endStep();
  browserMob.endTransaction();
} 
catch (e) {
  throw e;
}