import os
import sys
import time
import subprocess
import signal
import threading
import queue
import psutil


def enqueue_output(out, queue):
    for line in iter(out.readline, ''):
        queue.put(line)
    out.close()


def run_program(app_path, args, output_path, debug_name):
    log_file = os.path.join(output_path, debug_name + "_log.txt")

    if not os.path.exists(output_path):
        os.makedirs(output_path)

    cmd = [app_path] + list(args)
    print(f'Run command: {cmd} at {os.getcwd()}')

    process = subprocess.Popen(cmd,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               text=True)
    pid = process.pid

    def cleanup(signum, frame):
        print(f"Caught signal {signum}, stopping process with PID {pid}.")
        if psutil.pid_exists(pid):
            process.terminate()
        sys.exit(1)

    signal.signal(signal.SIGINT, cleanup)

    q_stdout = queue.Queue()
    q_stderr = queue.Queue()

    stdout_thread = threading.Thread(target=enqueue_output,
                                     args=(process.stdout, q_stdout))
    stderr_thread = threading.Thread(target=enqueue_output,
                                     args=(process.stderr, q_stderr))
    stdout_thread.daemon = True
    stderr_thread.daemon = True
    stdout_thread.start()
    stderr_thread.start()

    with open(log_file, 'w') as log:
        while process.poll() is None:  # Check if the process has terminated
            while not q_stdout.empty():
                line = q_stdout.get_nowait()
                print(line, end='')
                log.write(line)

            while not q_stderr.empty():
                line = q_stderr.get_nowait()
                print(line, end='', file=sys.stderr)
                log.write(line)

            time.sleep(0.1)

        # Read remaining output after the process has terminated
        while not q_stdout.empty():
            line = q_stdout.get_nowait()
            print(line, end='')
            log.write(line)

        while not q_stderr.empty():
            line = q_stderr.get_nowait()
            print(line, end='', file=sys.stderr)
            log.write(line)

    process.wait()

    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, cmd)
