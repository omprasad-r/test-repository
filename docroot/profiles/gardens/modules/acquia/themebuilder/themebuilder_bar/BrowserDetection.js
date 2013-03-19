/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global ThemeBuilder: true debug: true window: true*/


/**
 * This class parses the userAgent string to determine which browser is being
 * used.  This class was written because the browser support in jquery is
 * deprecated and has a bug in which some installations of IE8 report
 * compatibility with IE8 and IE6, and the version from jquery's browser
 * object is 6 rather than 8.
 *
 * Further, this object gives us the ability to apply the detection to a
 * specified userAgent string rather than getting it from the browser.  This
 * will help support browsers that we don't even have access to by verifying
 * the user agent string is parsed and taken apart correctly.
 * @class
 */
ThemeBuilder.BrowserDetect = ThemeBuilder.initClass();

/**
 * Instantiates a new instance of the BrowserDetect class with the specified
 * string.  If no string is specified, the navigator.userAgent will be used
 * instead.
 *
 * After instantiation, the caller can look at the browser, version, and OS
 * fields to determine the browser specifics.
 *
 * @param {String} userAgent
 *   (Optional) The user agent string to apply detection to.
 */
ThemeBuilder.BrowserDetect.prototype.initialize = function (userAgent) {
  this.userAgent = userAgent || navigator.userAgent;
  this._populateData();
  this.browser = this._searchString(this.dataBrowser) || "An unknown browser";
  this.version = this._searchVersion(this.userAgent) ||
    this._searchVersion(navigator.appVersion) || "an unknown version";
  this.OS = this._searchString(this.dataOS) || "an unknown OS";
};

ThemeBuilder.BrowserDetect.prototype._searchString = function (data) {
  for (var i = 0; i < data.length; i++)	{
    var dataString = data[i].string;
    var dataProp = data[i].prop;
    this.versionSearchString = data[i].versionSearch || data[i].identity;
    if (dataString) {
      if (dataString.indexOf(data[i].subString) !== -1) {
        return data[i].identity;
      }
    }
    else if (dataProp) {
      return data[i].identity;
    }
  }
};
  
ThemeBuilder.BrowserDetect.prototype._searchVersion = function (dataString) {
  var version = null;
  var index = dataString.indexOf(this.versionSearchString);
  if (index > -1) {
    version = parseFloat(dataString.substring(index + this.versionSearchString.length + 1));
    if (isNaN(version)) {
      version = null;
    }
  }
  return version;
};

ThemeBuilder.BrowserDetect.prototype._populateData = function () {
  this.dataBrowser = [
    {
      string: this.userAgent,
      subString: "Chrome",
      identity: "Chrome"
    },
    {
      string: this.userAgent,
      subString: "OmniWeb",
      versionSearch: "OmniWeb/",
      identity: "OmniWeb"
    },
    {
      string: navigator.vendor,
      subString: "Apple",
      identity: "Safari",
      versionSearch: "Version"
    },
    {
      string: this.userAgent,
      prop: window.opera,
      identity: "Opera"
    },
    {
      string: navigator.vendor,
      subString: "iCab",
      identity: "iCab"
    },
    {
      string: navigator.vendor,
      subString: "KDE",
      identity: "Konqueror"
    },
    {
      string: this.userAgent,
      subString: "Firefox",
      identity: "Firefox"
    },
    {
      string: navigator.vendor,
      subString: "Camino",
      identity: "Camino"
    },
    { // for newer Netscapes (6+)
      string: this.userAgent,
      subString: "Netscape",
      identity: "Netscape"
    },
    {
      string: this.userAgent,
      subString: "MSIE",
      identity: "Explorer",
      versionSearch: "MSIE"
    },
    {
      string: this.userAgent,
      subString: "Gecko",
      identity: "Mozilla",
      versionSearch: "rv"
    },
    { // for older Netscapes (4-)
      string: this.userAgent,
      subString: "Mozilla",
      identity: "Netscape",
      versionSearch: "Mozilla"
    }
  ];
  this.dataOS = [
    {
      string: navigator.platform,
      subString: "Win",
      identity: "Windows"
    },
    {
      string: navigator.platform,
      subString: "Mac",
      identity: "Mac"
    },
    {
      string: this.userAgent,
      subString: "iPhone",
      identity: "iPhone/iPod"
    },
    {
      string: navigator.platform,
      subString: "Linux",
      identity: "Linux"
    }
  ];
};
