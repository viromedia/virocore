<?cs

# print out the yaml nav for the reference docs, only printing the title,
path, and status_text (API level) for each package.

?><?cs
if:book.root ?><?cs var:book.root ?>:<?cs
else ?>reference:<?cs
/if?><?cs
if:docs.packages.link ?>
- title: Class Index
  path: /<?cs var:docs.classes.link ?><?cs
    if:dac ?>
  status_text: no-toggle<?cs
    /if?>
- title: Package Index
  path: /<?cs var:docs.packages.link ?><?cs
    if:dac ?>
  status_text: no-toggle<?cs
    /if?><?cs
/if ?><?cs
each:page = docs.pages?><?cs
  if:page.type == "package"?>
- title: <?cs var:page.label ?>
  path: /<?cs var:page.link ?><?cs
    if:dac ?>
  status_text: apilevel-<?cs var:page.apilevel ?><?cs
    /if?><?cs
  /if?><?cs
/each ?>
