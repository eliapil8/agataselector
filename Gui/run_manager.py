import os
import re
import argparse
from typing import Dict, List


class ReplayConfReader:
    def __init__(self, config_file: str, base_dir: str):
        self.config_file = config_file
        self.base_dir = base_dir
        self.replay_conf = {}
        self._parse_config()

    def _parse_config(self):
        """Parse the REPLAY_CONF section of the configuration file."""

        if not os.path.exists(os.path.join(self.base_dir, self.config_file)):
            raise FileNotFoundError(f"Configuration file not found: {self.config_file}")
        try:
            with open(os.path.join(self.base_dir, self.config_file), 'r') as file:
                lines = file.readlines()

            replay_conf_section = False
            for line in lines:
                # Strip comments and whitespace
                line = line.split('#')[0].strip()
                if not line:
                    continue
                if line == "REPLAY_CONF":
                    replay_conf_section = True
                    continue
                if replay_conf_section and line.startswith("#"):
                    continue
                if replay_conf_section and not line:
                    # End of section
                    break
                if replay_conf_section:
                    key, *value = line.split()
                    self.replay_conf[key] = " ".join(value)
        except Exception as e:
            raise RuntimeError(f"Error parsing configuration file: {e}")

    def find_matching_files(self) -> Dict[str, Dict[str, any]]:
        self._parse_config()
        if not os.path.exists(os.path.join(self.base_dir, self.config_file)):
            raise FileNotFoundError(f"Configuration file not found: {self.config_file}")
        """Find directories and files matching the replay configuration."""
        in_path = os.path.join(self.base_dir, self.replay_conf.get("IN_PATH", "."))
        in_sub_path = self.replay_conf.get("IN_SUB_PATH", "")
        replay_dir_pattern = self.replay_conf.get("REPLAY_DIR_PATTERN", "run_")
        in_file_pattern = self.replay_conf.get("IN_FILE_PATTERN", "Tree_")
        out_path = os.path.join(self.base_dir, self.replay_conf.get("OUT_PATH", "."))
        out_file_pattern = self.replay_conf.get("OUT_FILE_PATTERN", "run_")
        sum_file_pattern = self.replay_conf.get("SUM_FILE_PATTERN", "sum")

        file_data = {}

        # Look for directories matching the replay pattern
        for root, dirs, _ in os.walk(in_path):
            for directory in dirs:
                if re.match(replay_dir_pattern + r"\d+", directory):
                    run_number = directory.replace(replay_dir_pattern, "")
                    full_dir_path = os.path.join(root, directory, in_sub_path.lstrip('/'))
                    if os.path.exists(full_dir_path) and os.path.isdir(full_dir_path):
                        # Count input .root files
                        root_files = [
                            file for file in os.listdir(full_dir_path)
                            if re.match(in_file_pattern + r"\d+\.root", file)
                        ]

                        # Check for output files
                        output_dir = os.path.join(out_path)
                        output_files = [
                            file for file in os.listdir(output_dir)
                            if re.match(fr"{out_file_pattern}{run_number}_\d+\.root", file)
                        ]

                        # Check for sum file
                        sum_file = None
                        sum_subfile_count = 0
                        for file in os.listdir(output_dir):
                            try:
                                run_number_no_zeros = int(run_number)
                            except ValueError:
                                continue
                            match = re.match(fr"{sum_file_pattern}-{run_number_no_zeros}_(\d+)\.root", file)
                            if match:
                                sum_file = os.path.join(out_path, match.string)
                                sum_subfile_count = int(match.group(1))
                                break


                        # Store in dictionary
                        file_data[run_number] = {
                            "input_files_count": len(root_files),
                            "output_files_count": len(output_files),
                            "sum_file_present": sum_subfile_count,
                            "sum_file_name": sum_file
                        }
            # Stop walking deeper after the first level (optional)
            break

        return file_data




#if __name__ == "__main__":
#    reader = ReplayConfReader("selector.conf", "../build")
#    run_file_counts = reader.find_matching_files()
#    print("Run File Counts:")
#    for run, data in run_file_counts.items():
#        print(
#            f"Run {run}: {data['input_files_count']} input file(s), "
#            f"{data['output_files_count']} output file(s), "
#            f"Sum file present: {data['sum_file_present']}"
#        )

