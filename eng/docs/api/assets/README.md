# Doxygen Template for C++ API Documentation

When upgrading Doxygen, the following customizations have been made. 

## Export Doxygen Templates

Doxygen templates have several parts:

1. Doxygen Layout file `DoxygenLayout.xml`
1. Template files:
    1. `header.html`
    1. `footer.html`
    1. `style.css` (renamed from `customdoxygen.css`)

Further documentation on customizing Doxygen can be found here: https://www.doxygen.nl/manual/customize.html

To export the layout file:

```powershell
cd eng\docs\api\assets
doxygen -l
```

To export the HTML/CSS files:

```powershell
cd eng\docs\api\assets
doxygen -w html header.html footer.html style.css
```

## Configure templates

### DoxygenLayout.xml

Around line 6, change:

```xml
<tab type="pages" visible="yes" title="" intro=""/>
```

To read:

```xml
<tab type="pages" visible="yes" title="Concepts and Related Pages" intro=""/>
```

### header.html

In the `<head>` of the page add the following snippets:

Add Google analytics tag:

```html
<!-- Global site tag (gtag.js) - Google Analytics -->
<script async src="https://www.googletagmanager.com/gtag/js?id=UA-62780441-44"></script>
<script>
    window.dataLayer = window.dataLayer || [];
    function gtag(){dataLayer.push(arguments);}
    gtag('js', new Date());

    gtag('config', 'UA-62780441-44');
</script>
```

Add logic that populates and handles interactions with the version dropdown:

```html

<script>
WINDOW_CONTENTS = window.location.href.split("/");

// language specific
SELECTED_LANGUAGE = "cpp";
populateOptions("#versionSelector", [
  "#versionSelector",
  "#versionSelectorHeader"
]);

function currentVersion() {
  // Filled in by Doxygen template
  return "$projectnumber";
}

function currentPackage() {
  // Filled in by Doxygen template
  return "$projectname";
}

function httpGetAsync(targetUrl, callback) {
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function() {
    if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
      callback(xmlHttp.responseText);
  };
  xmlHttp.open("GET", targetUrl, true); // true for asynchronous
  xmlHttp.send(null);
}

function showSelectors(selectors) {
  selectors.forEach(function(item, index) {
    $(item).show();
  });
}

function hideSelectors(selectors) {
  selectors.forEach(function(item, index) {
    $(item).hide();
  });
}

function populateOptions(optionSelector, otherSelectors) {
  if (currentPackage()) {
    var versionRequestUrl =
      "https://azuresdkdocs.z19.web.core.windows.net" +
      SELECTED_LANGUAGE +
      "/" +
      currentPackage() +
      "/versioning/versions";

    httpGetAsync(versionRequestUrl, function(responseText) {
      if (responseText) {
        let options = responseText.match(/[^\r\n]+/g);
        populateVersionDropDown(optionSelector, options);
        showSelectors(otherSelectors);

        $(optionSelector).change(function() {
          targetVersion = $(this).val();
          window.location.href = getPackageUrl(SELECTED_LANGUAGE, currentPackage(), targetVersion);
        });
      }
    });
  }
}

function populateVersionDropDown(selector, values) {
  var select = $(selector);

  $("option", select).remove();

  $.each(values, function(index, text) {
    $("<option/>", { value: text, text: text }).appendTo(select);
  });
  select.val(currentVersion());
}

function getPackageUrl(language, package, version) {
  return (
    "https://azuresdkdocs.z19.web.core.windows.net" +
    language +
    "/" +
    package +
    "/" +
    version +
    "/index.html"
  );
}

</script>
```

For the following look for and replace the sections bounded by
`<!--BEGIN XXXX-->` and `<!--END XXXX-->` with the following:

In the project name section add the version selection dropdown in the
PROJECT_NUMBER section:

```html
  <!--BEGIN PROJECT_NAME-->
  <td id="projectalign">
   <div id="projectname">
    <div class="navbrand">

    </div>
    $projectname

    <!--BEGIN PROJECT_NUMBER-->
    <span id="projectnumber">
      <span id="versionSelectorHeader">
        <span>
            <select id="versionSelector">
                <option>$projectnumber</option>
            </select>
        </span>
      </span>
    </span>
    <!--END PROJECT_NUMBER-->
   </div>
   <!--BEGIN PROJECT_BRIEF--><div id="projectbrief">$projectbrief</div><!--END PROJECT_BRIEF-->
  </td>
  <!--END PROJECT_NAME-->
```

### footer.html

No changes

### style.css

Stylesheet changes may require more work to properly incoporate into the layout.
To be successful here when iterating through changes, make changes in the
browser's "inspect element" tools and then copy those changes to the stylesheet
file. Here are the most obvious changes:

Add these variables:

```css
    --title-foreground-color: white;
```

Change the title background color:

```css
    --title-background-color: rgb(0, 113, 197);
```

Change `#projectlogo`:

```css
    #projectlogo
    {
        text-align: center;
        vertical-align: bottom;
        border-collapse: separate;
        padding-left: 8px;
    }
```

Change `#projectname`:

```css
    #projectname
    {
        font-size: 200%;
        font-family: var(--font-family-title);
        margin: 0px;
        padding: 2px 0px;
        color: var(--title-foreground-color);

        #versionSelector {
            font-size: 24px;
        }
    }
```

## How it's wired up

See `cmake-modules/AzureDoxygen.cmake` to see how the layout files and and
templates are incorporated into the doxygen build. The build uses `logo.svg`
from `eng/common/docgeneration` instead of the repo-specific folder in
`eng/docs/api/assets`.
