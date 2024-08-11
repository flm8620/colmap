import os
from run_program import run_program


def main():
    root_path = "/home/norman/host_home/moving_norman_debug_colmap/"
    database_path = os.path.join(root_path, "distorted/database.db")
    debug_path = "/home/norman/host_home/moving_norman_debug_colmap/debug_viz"
    image_path = os.path.join(root_path, "input")
    mapper_output_path = os.path.join(root_path, "distorted/sparse/")
    camera_model = "OPENCV"
    camera_params = "2538.77, 2538.40, 2062.41, 1523.41, -0.15513205887, 0.1410179686,-0.000314, -0.0001957"
    common_args = [
        "--log_to_stderr", "true", "--log_level", "0", "--random_seed", "0"
    ]

    def run_feature():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        feature_args = [
            "--database_path",
            database_path,
            "--image_path",
            image_path,
            "--ImageReader.single_camera",
            "true",
            "--ImageReader.camera_model",
            camera_model,
            "--ImageReader.camera_params",
            camera_params,
            "--SiftExtraction.estimate_affine_shape",
            "false",
            "--SiftExtraction.max_image_size",
            "3000",
            "--SiftExtraction.max_num_features",
            "30000",
            "--SiftExtraction.peak_threshold",
            "0.002",
            "--SiftExtraction.first_octave",
            "-1",
            "--SiftExtraction.num_octaves",
            "4",
            "--SiftExtraction.upright",
            "true",
            "--SiftExtraction.use_gpu",
            "false",
        ]
        args = ["feature_extractor"] + common_args + feature_args

        run_program(executable_path, args, debug_path, "feature")

    def run_match():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        match_args = [
            "--database_path",
            database_path,
            "--SequentialMatching.overlap",
            "3",
            "--SequentialMatching.quadratic_overlap",
            "false",
            "--SiftMatching.max_num_matches",
            "50000",
        ]
        args = [
            "sequential_matcher",
        ] + common_args + match_args

        # Start the process
        run_program(executable_path, args, debug_path, "match")

    def run_mapper():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        match_args = [
            "--database_path",
            database_path,
            "--image_path",
            image_path,
            "--output_path",
            mapper_output_path,
            "--Mapper.ba_refine_focal_length",
            "false",
            "--Mapper.ba_refine_principal_point",
            "false",
            "--Mapper.ba_refine_extra_params",
            "false",
            "--Mapper.ba_global_function_tolerance",
            "0.000001",
        ]
        args = [
            "mapper",
        ] + common_args + match_args

        # Start the process
        run_program(executable_path, args, debug_path, "mapper")

    def run_undistort():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        input_path = os.path.join(mapper_output_path, "0")
        match_args = [
            "--image_path",
            image_path,
            "--output_path",
            root_path,
            "--input_path",
            input_path,
            "--output_type",
            "COLMAP",
        ]
        args = [
            "image_undistorter",
        ] + common_args + match_args

        # Start the process
        run_program(executable_path, args, debug_path, "undistort")

    if os.path.exists(database_path):
        os.remove(database_path)

    run_feature()
    run_match()
    run_mapper()
    run_undistort()


if __name__ == "__main__":
    main()
