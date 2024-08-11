import numpy as np
import matplotlib.pyplot as plt
import os
import subprocess

executable_path = "/home/norman/colmap/build/src/colmap/exe/leman_read_test"
output_path = "/home/norman/host_home/moving_norman_debug_colmap/debug_viz"

args = [
    "--database",
    "/home/norman/host_home/moving_norman_debug_colmap/distorted/database2.db",
    "--image_path",
    "/home/norman/host_home/moving_norman_debug_colmap/input",
    "--output_path",
    output_path,
]

# Start the process
process = subprocess.Popen(
    [executable_path] + args,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,
    bufsize=1,  # Line-buffered output
    universal_newlines=True  # Ensure text mode
)

# Stream stdout and stderr
try:
    for stdout_line in iter(process.stdout.readline, ""):
        print("STDOUT:", stdout_line, end="")  # Print each line from stdout

    for stderr_line in iter(process.stderr.readline, ""):
        print("STDERR:", stderr_line, end="")  # Print each line from stderr

    process.stdout.close()
    process.stderr.close()

    # Wait for the process to complete and get the return code
    return_code = process.wait()

    if return_code != 0:
        print(f"Process exited with return code {return_code}")

except KeyboardInterrupt:
    process.kill()  # If the user interrupts the process, kill it

# Load the matrix
match_matrix = np.loadtxt(os.path.join(output_path, "match_matrix.csv"),
                          delimiter=',')

plt.imshow(match_matrix, cmap='hot', interpolation='nearest')
plt.colorbar()
plt.savefig(os.path.join(output_path, "match.png"),
            dpi=300,
            bbox_inches='tight')
