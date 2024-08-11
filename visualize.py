import numpy as np
import matplotlib.pyplot as plt
import os
import subprocess
from run_program import run_program

executable_path = "/home/norman/colmap/build/src/colmap/exe/leman_read_test"
output_path = "/home/norman/host_home/moving_norman_debug_colmap/debug_viz"

args = [
    "--database",
    "/home/norman/host_home/moving_norman_debug_colmap/distorted/database.db",
    "--image_path",
    "/home/norman/host_home/moving_norman_debug_colmap/input",
    "--output_path",
    output_path,
]

run_program(executable_path, args, output_path, "viz")

# Load the matrix
match_matrix = np.loadtxt(os.path.join(output_path, "match_matrix.csv"),
                          delimiter=',')

plt.imshow(match_matrix, cmap='hot', interpolation='nearest')
plt.colorbar()
plt.savefig(os.path.join(output_path, "match.png"),
            dpi=300,
            bbox_inches='tight')
