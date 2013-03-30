/**
 * Prevent Drupal Gardens pages from being served from an iframe outside of our
 * servers.
 *
 * This script compares the location.host value of the window with the
 * location.host value of the window's parent. When a page is loaded in the DOM
 * window, window === window.parent. When a page is loaded in an iframe, then
 * window !== window.parent. We cannot trust this simple comparison, however,
 * since Drupal loads pages in iframes for the overlay and media dialogs, to
 * name two instances. So window.location.host is compared to
 * window.parent.location.host to determine if the page in the iframe originated
 * in the same domain as the parent window. If now, we set the location of the
 * parent window to the location of our hijacked page in the iframe, essentially
 * making it really difficult to steal our content.
 */

// Determine if this script is loaded inside an iframe.
if (window.location.host !== window.parent.location.host) {
  // Pop out of the iframe if this site is hijacked inside an iframe.
  window.top.location = window.self.location;
}
