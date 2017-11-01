<?cs # Create a comma separated list of annotations on obj that were in showAnnotations in Doclava ?>
<?cs # pre is an HTML string to start the list, post is an HTML string to close the list ?>
<?cs # for example call:show_annotations_list(cl, "<td>Annotations: ", "</td>") ?>
<?cs # if obj has nothing on obj.showAnnotations, nothing will be output ?>
<?cs def:show_annotations_list(obj) ?>
    <?cs each:anno = obj.showAnnotations ?>
      <?cs if:first(anno) ?>
        <span class='annotation-message'>
          Included in documentation by the annotations:
      <?cs /if ?>
      @<?cs var:anno.type.label ?>
      <?cs if:last(anno) == 0 ?>
        , &nbsp;
      <?cs /if ?>
      <?cs if:last(anno)?>
        </span>
      <?cs /if ?>
    <?cs /each ?>
<?cs /def ?>

<?cs # Override default class_link_table to display annotations ?>
<?cs def:class_link_table(classes) ?>
  <?cs set:count = #1 ?>
  <table class="jd-sumtable-expando">
    <?cs each:cl=classes ?>
      <tr class="<?cs if:count % #2 ?>alt-color<?cs /if ?> api apilevel-<?cs var:cl.type.since ?>" >
        <td class="jd-linkcol"><?cs call:type_link(cl.type) ?></td>
        <td class="jd-descrcol" width="100%">
          <?cs call:short_descr(cl) ?>&nbsp;
          <?cs call:show_annotations_list(cl) ?>
        </td>
      </tr>
      <?cs set:count = count + #1 ?>
    <?cs /each ?>
  </table>
<?cs /def ?>

<?cs
# Prints a comma separated list of parameters with optional line breaks
?><?cs
def:parameter_list(params, linebreaks) ?><?cs
  each:param = params ?><?cs
      call:simple_type_link(param.type)?> <?cs
      var:param.name ?><?cs
      if: name(param)!=subcount(params)-1
        ?>, <?cs if:linebreaks
?>
                <?cs /if ?><?cs
      /if ?><?cs
  /each ?><?cs
/def ?><?cs

# Print output for aux tags that are not "standard" javadoc tags ?><?cs
def:aux_tag_list(tags) ?><?cs
  each:tag = tags ?><p><?cs
      if:tag.kind == "@memberDoc" ?><?cs call:tag_list(tag.commentTags) ?><?cs
      elif:tag.kind == "@paramDoc" ?><?cs call:tag_list(tag.commentTags) ?><?cs
      elif:tag.kind == "@returnDoc" ?><?cs call:tag_list(tag.commentTags) ?><?cs
      elif:tag.kind == "@range" ?><?cs call:dump_range(tag) ?><?cs
      elif:tag.kind == "@intDef" ?><?cs call:dump_int_def(tag) ?><?cs
      elif:tag.kind == "@permission" ?><?cs call:dump_permission(tag) ?><?cs
      elif:tag.kind == "@service" ?><?cs call:dump_service(tag) ?><?cs
      /if ?><?cs
  /each ?></p><?cs
/def ?><?cs

# Print output for @range tags ?><?cs
def:dump_range(tag) ?><?cs
  if:tag.from && tag.to ?>Value is between <?cs var:tag.from ?> and <?cs var:tag.to ?> inclusive.<?cs
  elif:tag.from ?>Value is <?cs var:tag.from ?> or greater.<?cs
  elif:tag.to ?>Value is <?cs var:tag.to ?> or less.<?cs
  /if ?><?cs
/def ?><?cs

# Print output for @intDef tags ?><?cs
def:dump_int_def(tag) ?><?cs
  if:tag.flag ?><?cs
    if:subcount(tag.values) > 1 ?>Value is either <code>0</code> or combination of <?cs
    else ?>Value is either <code>0</code> or <?cs
    /if ?><?cs
  else ?>Value is <?cs
  /if ?><?cs
  loop:i = #0, subcount(tag.values), #1 ?><?cs
    with:val = tag.values[i] ?><?cs
      call:tag_list(val.commentTags) ?><?cs
      if i == subcount(tag.values) - 2 ?> or <?cs
      elif:i < subcount(tag.values) - 2 ?>, <?cs
      /if ?><?cs
    /with ?><?cs
  /loop ?>.<?cs
/def ?><?cs

# Print output for @permission tags ?><?cs
def:dump_permission(tag) ?>Requires the <?cs
  loop:i = #0, subcount(tag.values), #1 ?><?cs
    with:val = tag.values[i] ?><?cs
      call:tag_list(val.commentTags) ?><?cs
      if i == subcount(tag.values) - 2 ?><?cs
        if tag.any ?> or <?cs
        else ?> and <?cs
        /if ?><?cs
      elif:i < subcount(tag.values) - 2 ?>, <?cs
      /if ?><?cs
    /with ?><?cs
  /loop ?><?cs
  if subcount(tag.values) > 1 ?> permissions.<?cs
  else ?> permission.<?cs
  /if ?><?cs
/def ?><?cs

# Print output for @service tags ?><?cs
def:dump_service(tag) ?>Instances of this class must be obtained using <?cs
  loop:i = #0, subcount(tag.values) - 1, #2 ?><?cs
    call:tag_list(tag.values[i].commentTags) ?> with the argument <?cs
    call:tag_list(tag.values[i+1].commentTags) ?><?cs
    if i < subcount(tag.values) - 2 ?> or <?cs
    /if ?><?cs
  /loop ?>.<?cs
/def ?>
