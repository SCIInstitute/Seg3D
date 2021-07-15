# Ir-tools

The ir-tools image registration command line tools ([ir-tools wiki](http://medimagingwiki.sci.utah.edu/wiki/Ir-tools), [more tool documentation](http://www.sci.utah.edu/~bgrimm/iris/docs/tools.html)) are available as Python commands in Seg3D. Seg3D needs to be built from source with the CMake option **BUILD_MOSAIC_TOOLS** turned on ([Seg3D build instructions](build.md#optional-image-registration-tools)).

## Commands in Python

The following commands can be run in Seg3D's python console. The layerid parameter is not used, and should be set to ***\'\<none\>\'***. The Seg3D GitHub repository has [sample scripts](https://github.com/SCIInstitute/Seg3D/tree/master/scripts).

* **ir-fft**: fftfilter( layerid=\<none\>, directory=\<directory\>, output_mosaic=\<output_mosaic\>, [tile_strategy=default], [min_peak=0.1], [peak_threshold=0.3], [images=[]], [shrink_factor=1], [num_threads=0], [pyramid_levels=1], [iterations_per_level=5], [pixel_spacing=1.0], [clahe_slope=1.0], [overlap_min=0.05], [overlap_max=1.0], [use_standard_mask=false], [try_refining=false], [run_on_one=false], [sandbox=-1] )

* **ir-assemble**: assemblefilter( layerid=\<none\>, input_mosaic=\<input_mosaic\>, output_image=\<output_image\>, directory=\<directory\>, [shrink_factor=1], [num_threads=0], [tile_width=max], [tile_height=max], [pixel_spacing=1.0], [clahe_slope=1.0], [use_standard_mask=false], [save_int16_image=false], [save_uint16_image=false], [save_variance=false], [remap_values=false], [defer_image_loading=false], [feathering=none], [mask=<none>], [sandbox=-1] )

* **ir-refine-grid**: refinegridfilter( layerid=\<none\>, input_mosaic=\<input_mosaic\>, output_mosaic=\<output_mosaic\>, directory=\<directory\>, [shrink_factor=1], [num_threads=0], [iterations=10], [cell_size=0], [mesh_rows=0], [mesh_cols=0], [pixel_spacing=1.0], [displacement_threshold=1.0], [use_standard_mask=false], [sandbox=-1] )

* **ir-stos-brute**: slicetoslicebrutefilter( layerid=\<none\>, input_fixed=\<input_fixed\>, input_moving=\<input_moving\>, output_stos=\<output_stos\>, [shrink_factor=1], [pixel_spacing=1.0], [clahe_slope=1.0], [best_shift_x=0.0], [best_shift_y=0.0], [best_angle=0.0], [best_overlap=0.0], [use_standard_mask=false], [use_refinement=false], [use_cubic=false], [regularize=false], [flip_fixed=false], [flip_moving=false], [mask_fixed=\<none\>], [mask_moving=\<none\>], [image_dir_fixed=\<none\>], [image_dir_moving=\<none\>], [sandbox=-1] )

* **ir-clahe**: clahefilter( layerid=\<none\>, input_image=\<input_image\>, output_image=\<output_image\>, [max_slope=0], [shrink_factor=1], [window_x=1], [window_y=1], [remap_min=max], [remap_max=-max], [median_radius=0], [bins=256], [sigma=max], [use_standard_mask=false], [mask=\<none\>], [sandbox=-1] )

* **ir-refine-translate**: refinetranslatefilter( layerid=\<none\>, input_mosaic=\<input_mosaic\>, output_mosaic=\<output_mosaic\>, directory=\<directory\>, [shrink_factor=1], [num_threads=0], [prune_tile_size=32], [pixel_spacing=1.0], [intensity_tolerance=0], [max_offset_x=max], [max_offset_y=max], [black_mask_x=max], [black_mask_y=max], [use_standard_mask=false], [use_clahe=true], [sandbox=-1] )

* **ir-stos**: slicetoslicefilter( layerid=\<none\>, input_fixed=\<input_fixed\>, input_moving=\<input_moving\>, output_stos=\<output_stos\>, [shrink_factor=1], [descriptor_version=4], [pixel_spacing=1.0], [clahe_slope=1.0], [use_standard_mask=false], [flip_fixed=false], [flip_moving=false], [mask_fixed=\<none\>], [mask_moving=\<none\>], [image_dir_fixed=\<none\>], [image_dir_moving=\<none\>], [sandbox=-1] )

* **ir-stos-grid**: slicetoslicegridfilter( layerid=\<none\>, input_stos=\<input_stos\>, output_stos=\<output_stos\>, neighborhood=\<neighborhood\>, [shrink_factor=1], [iterations=1], [grid_spacing=0], [grid_rows=0], [grid_cols=0], [median_radius=1], [num_threads=0], [clahe_slope=1.0], [minimum_overlap=0.5], [displacement_threshold=1.0], [disable_fft=false], [slice_dir_fixed=\<none\>], [slice_dir_moving=\<none\>], [image_dir_fixed=\<none\>], [image_dir_moving=\<none\>], [sandbox=-1] )

* **ir-stom**: slicetovolumefilter( layerid=\<none\>, input_files=\<input_files\>, image_dirs=\<image_dirs\>, output_prefixes=\<output_prefixes\>, [slice_dirs=[]], [shrink_factor=1], [clahe_window=64], [left_padding=0], [top_padding=0], [right_padding=0], [bottom_padding=0], [tile_width=max], [tile_height=max], [clahe_slope=1.0], [visualize_warp=false], [use_base_mask=false], [preadded=false], [remap_values=false], [save_int16_image=false], [save_uint16_image=false], [image_extension=.tif], [load_volume=false], [sandbox=-1] )

* **ir-add-transforms**: addtransformsfilter( layerid=\<none\>, input_files=\<input_files\>, output_prefixes=\<output_prefixes\>, [slice_dirs=[]], [sandbox=-1] )

* **ir-blob**: blobfeatureenhancementfilter( layerid=\<none\>, input_image=\<input_image\>, output_image=\<output_image\>, [shrink_factor=1], [num_threads=0], [radius=2], [threshold=3], [median_radius=0], [use_standard_mask=false], [sandbox=-1] )
