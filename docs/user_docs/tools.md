# Seg3d Tools

 There are many different tools and filters implemented in Seg3D. These tools and filters are there to attempt several tasks, from reformatting original data, smoothing and de-noising image data, to automatic and manual segmenting. Every tool and filter will be describe briefly in an effort to provide the user with a list of methods that may be used to segment the data that they have. These methods have been separated into four categories, general tools, mask filters, data filters, and advanced filters.

Most of the tools and filters have a ‘replace’ option. The default for most of these functions is to create a new layer, but ‘replace’ will cause the new layer to instead replace the old one. This is very useful in avoiding the generation of multiple layer that are only intermediate steps. Also, most tools and filters will by default insert the active layer as the primary input. You may disable this and choose manually which layer to modify.

Many of the tools and filters must use a certain type of data as the input, either mask or image data. If the wrong type of layer is chosen as the input, the tool/filter interface will display a warning message in a red box near the execution button informing you of this. The tool will also be disabled until the correct type of layer is chosen. This second feature applies to empty inputs also.

A final note is that when tools have multiple inputs, the inputs must all be from the same layer group (geometric grid), with exception of the registration tool. This ensures that all the voxels in the multiple inputs have corresponding voxels to every other input volume.

{% comment %}from https://gist.github.com/pepelsbey/9334494{% endcomment %}
{% capture tmp %}
{% for page in site.pages %}{% if page.category == "ToolDocs" %} {{ page.tool}} {% endif %}{% endfor %}
{% endcapture %}

{% assign categories = tmp | split: ' ' %}
{% assign tmp = categories[0] %}

{% for cat in categories %}
  {% unless tmp contains cat %}
    {% capture tmp %}{{ tmp }} {{ cat }}{% endcapture %}
  {% endunless %}
{% endfor %}

{% assign toolcategories = tmp | split: ' ' %}

{% capture toolpages %}
  {% for cat in toolcategories %}?{{ cat }}
    {% for page in site.pages %}
      {% if page.tool == cat %}${{ page.title }}#{{ page.url | prepend: site.github.url }}
      {% endif %}
    {% endfor%}
  {% endfor %}
{% endcapture %}

{% assign sortedpages = toolpages | strip | strip_newlines | split: '?' | sort %}

{% for pagestring in sortedpages %}
  {% assign pageitems = pagestring | split: '$' %}
  {% if pageitems[0] %}
## {{ pageitems[0] }}
    {% for item in pageitems %}
      {% comment %}skip category list item (index 0){% endcomment %}
      {% if forloop.first %} {% continue %} {% endif%}
      {% assign linkitem = item | split: '#' %}
**[{{ linkitem[0] }}]({{ linkitem[1] }}){:target="_blank"}**
    {% endfor %}
  {% endif %}
{% endfor %}
