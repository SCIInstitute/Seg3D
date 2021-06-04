# 2D Slice Viewer

The first and second tables below show the mouse and keyboard actions which can be used to control the visualization and manipulation of image and segmentation data. Though these functions are general, there are tools which used these functions for specify purposes or may otherwise block a couple of these functions, the most prominent example is the scroll wheel in the paint brush tool is used to change brush size. In this case, you can still scroll through slices by holding Shift. In all cases, an alternative is given in the software. Also presented in this section is a list and description of the icons presented in the 2D slice viewer.

```eval_rst
.. list-table:: Mouse Functions in the 2D Viewers
   :widths: 30 70
   :width: 100%
   :header-rows: 1

   * - Mouse Command
     - Function

   * - left button drag
     - Modify brightness and contrast. Vertical is contrast, horizontal is brightness.

   * - scroll up/down
     - Move up/down a slice. Note: using Shift maybe needed while using some tools (like paint brush)

   * - CMD/CTRL+right button
     - Move slices in other planes to intersect at cursor. Viewers must have the picking icon enabled.

   * - Shift+left button drag
     - Pan view

   * - Shift+right button drag
     - Zoom view in/out

```


```eval_rst
.. list-table:: Keyboard Actions in the 2D Viewers
   :widths: 30 70
   :width: 100%
   :header-rows: 1

   * - Keyboard Action
     - Function

   * - up arrow, >
     - Move up one slice

   * - down arrow, <
     - Move down one slice

   * - Shift+up arrow, Shift+>
     - Jump up n slices (set n in preferences)

   * - Shift+down arrow, Shift+<
     - Jump down n slices (set n in preferences)

   * - left/right arrow
     - Change active layer to previous/next layer

   * - SPACE
     - Toggle layer visibility on/off (active layer)

   * - G
     - Toggle grid visibility

   * - P
     - Toggle picking state

   * - T
     - Toggle overlay visibility

   * - L
     - Toggle lock viewer (to other locked views of the same type)
```


```eval_rst
.. |AutoViewOff| image:: ../../../Seg3DBasicFunctionality_figures/AutoViewOff.png
.. |LockOff| image:: ../../../Seg3DBasicFunctionality_figures/LockOff.png
.. |VisibleOff| image:: ../../../Seg3DBasicFunctionality_figures/VisibleOff.png
.. |PickingOff| image:: ../../../Seg3DBasicFunctionality_figures/PickingOff.png
.. |GridOff| image:: ../../../Seg3DBasicFunctionality_figures/GridOff.png
.. |FlipHorizOff| image:: ../../../Seg3DBasicFunctionality_figures/FlipHorizOff.png
.. |FlipVertOff| image:: ../../../Seg3DBasicFunctionality_figures/FlipVertOff.png
.. |OverlayOff| image:: ../../../Seg3DBasicFunctionality_figures/OverlayOff.png
.. |PickingLinesOff| image:: ../../../Seg3DBasicFunctionality_figures/PickingLinesOff.png

.. list-table:: Icons and Actions in the 2D Viewers
   :widths: 10 90
   :width: 100%
   :header-rows: 1

   * - Icon
     - Function

   * - |AutoViewOff|
     - Autoview Icon: This icon forces the panel to fit the objects in viewer with maximum size.

   * - |LockOff|
     - Lock View Icon: This icon toggles the viewer lock on the panel (shortcut: L). Any changes to any locked viewers will change all the views. Viewers must be the same type and each must be locked to use this funtion.

   * - |VisibleOff|
     - Visibility Icon: This icon toggles the visibility of the plane in the 3D volume viewer.

   * - |PickingOff|
     - Picking Icon: This icon toggles the ability of other planes to pick the slice to view in the panel. Only one viewer can have this option enabled. If there is only one type of plane, this cannot be disabled.

   * - |GridOff|
     - Grid Icon: This icon toggles the visibility of the grid in the viewer

   * - |FlipHorizOff|
     - Flip Horizontal Icon: This icon will horizontally flip the visualization of the slices in the viewer.

   * - |FlipVertOff|
     - Flip Vertical Icon: This icon will vertically flip the visualization of the slices in the viewer.

   * - |OverlayOff|
     - Overlay Icon: This icon will toggle the visibility of the overlay on the viewer. This will allow unobstructed viewing of the slices.

   * - |PickingLinesOff|
     - Picking Lines Icon: This icon toggles the visibility of the picking lines (shows other slices) in the viewing panel.

```
