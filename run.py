import os
from run_program import run_program


def main():
    database_path = "/home/norman/host_home/moving_norman_debug_colmap/distorted/database.db"
    image_path = "/home/norman/host_home/moving_norman_debug_colmap/input"
    camera_model = "OPENCV"
    camera_params = "2538.77, 2538.40, 2062.41, 1523.41, -0.15513205887, 0.1410179686,-0.000314, -0.0001957"
    common_args = [
        "--log_to_stderr", "true", "--log_level", "0", "--random_seed", "0",
        "--database_path", database_path
    ]

    def run_feature():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        output_path = "/home/norman/host_home/moving_norman_debug_colmap/debug_viz"
        feature_args = [
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

        run_program(executable_path, args, output_path, "feature")

    def run_match():
        executable_path = "/home/norman/colmap/build/src/colmap/exe/colmap"
        output_path = "/home/norman/host_home/moving_norman_debug_colmap/debug_viz"
        match_args = [
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
        run_program(executable_path, args, output_path, "match")

    if os.path.exists(database_path):
        os.remove(database_path)

    run_feature()
    run_match()


if __name__ == "__main__":
    main()
