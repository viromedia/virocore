<?cs def:body_content_wrap_start() ?>
  <div class="wrap clearfix" id="body-content">
<?cs /def ?><?cs

def:fullpage() ?>
  <div id="body-content">
    <div>
<?cs /def ?><?cs

# The default side navigation for the reference docs ?><?cs
def:reference_default_nav() ?>
  <!-- Fullscreen toggler -->
  <button data-fullscreen class="dac-nav-fullscreen">
    <i class="dac-sprite dac-fullscreen"></i>
  </button>

  <script>$('[data-fullscreen]').dacFullscreen();</script>
  <!-- End: Fullscreen toggler -->

  <?cs if:reference.gcm || reference.gms ?>
    <?cs include:"../../../../frameworks/base/docs/html/google/google_toc.cs" ?>
    <script type="text/javascript">
      showGoogleRefTree();
    </script>
  <?cs else ?>
    <div id="devdoc-nav">
      <div id="api-nav-header">
        <div id="api-nav-title">Viro APIs</div>
      </div><!-- end nav header -->
      <script>
        var SINCE_DATA = [ <?cs
          each:since = since ?>'<?cs
            var:since.name ?>'<?cs
            if:!last(since) ?>, <?cs /if ?><?cs
            /each
          ?> ];
        buildApiLevelSelector();
      </script>

      <div class="dac-reference-nav" data-reference-tree>
        <ul class="dac-reference-nav-list" data-reference-namespaces>
          <?cs call:package_link_list(docs.packages) ?>
        </ul>

        <?cs if:subcount(class.package) ?>
        <ul data-reference-resources>
          <?cs call:list("Annotations", class.package.annotations) ?>
          <?cs call:list("Interfaces", class.package.interfaces) ?>
          <?cs call:list("Classes", class.package.classes) ?>
          <?cs call:list("Enums", class.package.enums) ?>
          <?cs call:list("Exceptions", class.package.exceptions) ?>
          <?cs call:list("Errors", class.package.errors) ?>
        </ul>
        <?cs elif:subcount(package) ?>
        <ul data-reference-resources>
          <?cs call:class_link_list("Annotations", package.annotations) ?>
          <?cs call:class_link_list("Interfaces", package.interfaces) ?>
          <?cs call:class_link_list("Classes", package.classes) ?>
          <?cs call:class_link_list("Enums", package.enums) ?>
          <?cs call:class_link_list("Exceptions", package.exceptions) ?>
          <?cs call:class_link_list("Errors", package.errors) ?>
        </ul>
        <?cs /if ?>
      </div>
    </div>
  <?cs /if ?>
<?cs /def ?><?cs

def:ndk_nav() ?>
  <div class="wrap clearfix" id="body-content"><div class="cols">
    <div class="col-3 dac-toggle dac-mobile" id="side-nav" itemscope itemtype="http://schema.org/SiteNavigationElement">
      <?cs call:mobile_nav_toggle() ?>
      <div class="dac-toggle-content" id="devdoc-nav">
        <div class="scroll-pane">
<?cs
if:guide ?><?cs include:"../../../../frameworks/base/docs/html/ndk/guides/guides_toc.cs" ?><?cs
elif:reference ?><?cs include:"../../../../frameworks/base/docs/html/ndk/reference/reference_toc.cs" ?><?cs
elif:downloads ?><?cs include:"../../../../frameworks/base/docs/html/ndk/downloads/downloads_toc.cs" ?><?cs
elif:samples ?><?cs include:"../../../../frameworks/base/docs/html/ndk/samples/samples_toc.cs" ?><?cs
/if ?>
        </div>
      </div>
    </div> <!-- end side-nav -->
<?cs /def ?><?cs

def:header_search_widget() ?>
  <form data-search class="dac-header-search">
    <button class="dac-header-search-close" data-search-close>
      <i class="dac-sprite dac-back-arrow"></i>
    </button>

    <div class="dac-header-search-inner">
      <i class="dac-sprite dac-search-white dac-header-search-icon"></i>
      <input id="search_autocomplete" type="text" value="" autocomplete="off" name="q" class="dac-header-search-input" placeholder="Search" />
      <button class="dac-header-search-clear dac-hidden" data-search-clear>
        <i class="dac-sprite dac-close-black"></i>
      </button>
    </div>
  </form>
<?cs /def ?><?cs

def:search_results() ?>
  <div id="search-results" class="dac-search-results">
    <div id="dac-search-results-history" class="dac-search-results-history">
      <div class="wrap dac-search-results-history-wrap">
        <div class="cols">
          <div class="col-1of2 col-tablet-1of2 col-mobile-1of1">
            <h2>Most visited</h2>
            <div class="resource-flow-layout" data-history-query="history:most/visited" data-maxresults="3" data-cardsizes="18x2"></div>
          </div>

          <div class="col-1of2 col-tablet-1of2 col-mobile-1of1">
            <h2>Recently visited</h2>
            <div class="resource-flow-layout cols" data-history-query="history:recent" data-allow-duplicates="true" data-maxresults="3" data-cardsizes="18x2"></div>
          </div>
        </div>
      </div>
    </div>

    <div id="dac-search-results-content" class="dac-search-results-content">
      <div class="dac-search-results-metadata wrap">
        <div class="dac-search-results-for">
          <h2>Results for <span id="search-results-for"></span></h2>
        </div>

        <div id="dac-search-results-hero"></div>

        <div class="dac-search-results-hero cols">
          <div id="dac-search-results-reference" class="col-3of6 col-tablet-1of2 col-mobile-1of1">
            <div class="suggest-card reference no-display">
              <ul class="dac-search-results-reference">
              </ul>
            </div>
          </div>
          <div id="dac-custom-search-results"></div>
        </div>
      </div>

    </div>
  </div>
<?cs /def ?><?cs

def:custom_left_nav() ?>
  <?cs if:(!fullpage && !nonavpage) || forcelocalnav ?>
    <?cs if:!referenceonly ?>
    <a class="dac-nav-back-button dac-swap-section dac-up dac-no-anim" data-swap-button href="javascript:;">
      <i class="dac-sprite dac-nav-back"></i> 
    </a>
    <?cs /if ?>
    <div class="dac-nav-sub dac-swap-section dac-right dac-active" itemscope
      itemtype="http://schema.org/SiteNavigationElement" <?cs
        if:referenceonly ?>style="top:0 !important;"<?cs /if ?>>
      <?cs call:reference_default_nav() ?>
    </div>
  <?cs /if ?>
<?cs /def ?><?cs

# appears at the bottom of every page ?><?cs
def:custom_cc_copyright() ?>
  Except as noted, this content is
  licensed under <a href="http://creativecommons.org/licenses/by/2.5/">
  Creative Commons Attribution 2.5</a>. For details and
  restrictions, see the <a href="<?cs var:toroot ?>license.html">Content
  License</a>.
<?cs /def ?><?cs

def:custom_copyright() ?>
  Copyright © 2017 Viro Media. All rights reserved.
<?cs /def ?><?cs

def:custom_footerlinks() ?>
  <a href="<?cs var:toroot ?>about/android.html">About Android</a>
  <a href="<?cs var:toroot ?>auto/index.html">Auto</a>
  <a href="<?cs var:toroot ?>tv/index.html">TV</a>
  <a href="<?cs var:toroot ?>wear/index.html">Wear</a>
  <a href="<?cs var:toroot ?>legal.html">Legal</a>
<?cs /def ?><?cs

# appears on the right side of the blue bar at the bottom off every page ?><?cs
def:custom_buildinfo() ?>
  <?cs if:!google && !reference.gcm && !reference.gms ?>
    Android <?cs var:sdk.version ?>&nbsp;r<?cs var:sdk.rel.id ?> &mdash;
  <?cs /if ?>
  <script src="<?cs var:toroot ?>timestamp.js" type="text/javascript"></script>
  <script>document.write(BUILD_TIMESTAMP)</script>
<?cs /def ?>
