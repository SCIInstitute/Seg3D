# 3D Volume Viewer

Though there is no segmentation that can be performed in the 3D volume viewer, it is a very useful function in Seg3D. It allows the user to see the 3D representation of the original and segmented data. There are many objects that can be viewed in the 3D viewer such as the 2D slices in 3D, isosurfaces of the segmented data, volume rendering of the image data, depth cues, and clipping planes. The purpose of this viewer is to allow you to view your data in as many ways as possible to facilitate segmentation.

| **Mouse Command** | **Function** <a name="3dmouse"></a>|
|:----|:----|
| left button drag               | Pan scene                                   |
| middle button drag             | Rotate scene                                |
| right button drag              | Zoom in/out on scene                        |

Table 3.4: List of mouse functions in the 3D volume viewer

| **Keyboard Action** | **Function** <a name="2dkey"></a> |
|:----|:----|
| H                                | Toggle volume lighing                                       |
| I                                | Toggle isosurface visibility                                |
| T                                | Toggle overlay visibility                                   |
| L                                | Toggle lock viewer (to other locked views of the same type) |

Table 3.5: List of keyboard actions in the 3D volume viewer

| **Icon** | **Function** <a name="3dicons"></a> |
|:----|:----|
|![alt text](../Seg3DBasicFunctionality_figures/AutoViewOff.png)              | Autoview Icon: This icon forces the panel to fit the objects in viewer with maximum size.|
|![alt text](../Seg3DBasicFunctionality_figures/LockOff.png)                  | Lock View Icon: This icon toggles the viewer lock on the panel (shortcut: L). Any changes to any locked viewers will change all the views. Viewers must be the same type and each must be locked to use this funtion.|
|![alt text](../Seg3DBasicFunctionality_figures/AlignOff.png)                 | Snap to Axis Icon: This icon will move the scene so that the viewing angle is aligned with the nearest axis. This will effectively “straighten” the scene.|
|![alt text](../Seg3DBasicFunctionality_figures/LightOff.png)                 | Lighting Icon: This icon toggles the lighting in the 3D viewer. With the lighting disable, the images will be shaded as if they were flat, i.e., no shading.|
|![alt text](../Seg3DBasicFunctionality_figures/FogOff.png)                   | Depth Cue Icon: This icon toggles the depth cue for the 3d viewer. This depth cue acts as fog, blending objects further from the camera into the background. Fog parameters can be changed in the volume view window ([Sec. 4.4.1](FogPanel.md)).|
|![alt text](../Seg3DBasicFunctionality_figures/ClipOff.png)                  | Clipping Icon: This icon toggles the viewing of the clipping planes in the 3D viewer. Clipping planes must be created before they can be seen in the volume viewer ([Sec. 4.4.2](ClippingPlanesPanel.md)). |
|![alt text](../Seg3DBasicFunctionality_figures/IsosurfaceVisibleOff.png)     | Isosurface Visibility Icon: This icon toggles visibility of the isosurfaces in the 3D viewer. This functions effects all the isosurfaces are declared visible in the layer manager. |
|![alt text](../Seg3DBasicFunctionality_figures/SlicesVisibleOff.png)         | Slice Visibility Icon: This icon toggles visibility of the slices in the 3D viewer.|
|![alt text](../Seg3DBasicFunctionality_figures/InvisibleSlicesVisibleOff.png)| Invisible Slice Visibility Icon: This icon toggles visibility of the invisible slices in the 3D viewer. Invisible slices are left when 2D viewers are destroyed. |
|![alt text](../Seg3DBasicFunctionality_figures/VolumeRenderingOff.png)       | Volume Rendering Visibility Icon: This icon toggles visibility of the Volume Rendering in the 3D viewer. Volume Rendering must first be created in the Volume View window ([Sec. 4.4.3](VolumeRenderingPanel.md))|
|![alt text](../Seg3DBasicFunctionality_figures/VolumeVisibleOff.png)         | Volume visibility Icon: This icon toggles visibility of the borders of the active layer in the 3D viewer.                                                            |
|![alt text](../Seg3DBasicFunctionality_figures/OverlayOff.png)               | Overlay Icon: This icon will toggle the visibility of the overlay on the viewer. This will allow unobstructed viewing of the scene.                     |

Table 3.6: List of icons and actions in the 3D viewer
