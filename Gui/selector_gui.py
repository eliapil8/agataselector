import sys
import subprocess
from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QLineEdit, QPushButton, QFileDialog, QWidget, QTextEdit, QCheckBox, QSpinBox
from PyQt5.QtWidgets import QVBoxLayout, QTableWidget, QTableWidgetItem, QHBoxLayout, QComboBox, QProgressBar
from run_manager import ReplayConfReader
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import Qt

import re
import os
import shutil
import threading
from functools import partial

# Worker Thread for Running the Selector
class SelectorRunner(QThread):
    progress = pyqtSignal(str)  # Signal for progress updates
    thread_progress = pyqtSignal(int, int, str)  # Signal for thread-specific progress
    clear_progress_bars = pyqtSignal()  # Signal to clear progress bars
    
    def __init__(self, command, total_threads, total_runs, base_dir):
        super().__init__()
        self.command = command
        self.total_threads = total_threads
        self.total_runs = total_runs
        #self.thread_counters = {}  # Initialize counters for each thread
        self.base_dir= base_dir
        self.process = None

    def is_executable(file_name):
        # Check if the file is in the PATH and executable
        executable_path = shutil.which(file_name)
        if executable_path and os.access(executable_path, os.X_OK):
            return True
        return False

    def run(self):

        # Example usage
        self.process = subprocess.Popen(
                self.command,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                universal_newlines=True,
                cwd=self.base_dir
        )

        # Track progress for each line
        for line in self.process.stdout:
            self.progress.emit(line.strip())  # Emit line to the output display
            parts = re.split(r'(Thread nr.)', line)

            # Combine the delimiter back with the following part
            parts = [parts[i] + parts[i + 1] for i in range(1, len(parts), 2)]


            for part in parts:

                # First, search for the thread number with exactly 3 digits after the decimal
                match_threads = re.findall(r'Thread nr\. (\d+\.\d{3})', part)  # Find all thread numbers

                # If there are multiple threads, process each of them
                if match_threads:
                    for i, thread_number in enumerate(match_threads):
                        thread_id = thread_number.split(".")
                        thread_id = int(thread_id[0]) * self.total_threads +  int(thread_id[1])

                        match_progress = re.findall(r'(\d+\.\d+) %', part)  # Find all progress percentages
                        match_idle = re.findall(r'idle', part)  # Find all progress percentages
                        match_writing = re.findall(r'writing', part)  # Find all progress percentages

                        if match_progress:
                            percentage = float(match_progress[-1])
                            #self.thread_counters[thread_id] = percentage
                            self.thread_progress.emit(thread_id, int(percentage), thread_number)
                        if match_writing:
                            self.thread_progress.emit(thread_id, 100, thread_number)
                        if match_idle:
                            self.thread_progress.emit(thread_id, 0, thread_number)
                elif "Output file name of sum will be :" in part:
                    self.clear_progress_bars.emit()

        self.process.wait()

    def kill(self):
        """Terminate the subprocess if it is running."""
        if self.process and self.process.poll() is None:
            self.process.terminate()
            self.process.wait()

    def stop(self, path):
        """Create a stop file the subprocess if it is running."""
        if self.process and self.process.poll() is None:
            with open(os.path.join(path, "StopFile"), "w") as file:
                pass  # Do nothing, just create the file

class ProgressWindow(QMainWindow):
    def __init__(self, total_threads):
        super().__init__()
        self.setWindowTitle("Progress")
        self.setGeometry(100, 100, 400, 600)  # Adjust size as needed

        # Central widget for layout
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        self.layout = QVBoxLayout()
        central_widget.setLayout(self.layout)

        # Progress bars list
        self.progress_bars = []
        self.thread_labels = []
        self.percent_labels = []
        for _ in range(total_threads):
            self.thread_layout = QHBoxLayout()
            
            thread_label = QLabel("Thread id", self)
            self.thread_layout.addWidget(thread_label)
            self.thread_labels.append(thread_label)

            progress_bar = QProgressBar(self)
            progress_bar.setMaximum(100)
            self.thread_layout.addWidget(progress_bar)
            self.progress_bars.append(progress_bar)
            
            percent_label = QLabel("0 %", self)
            self.thread_layout.addWidget(percent_label)
            self.percent_labels.append(percent_label)

            self.layout.addLayout(self.thread_layout)

    def update_thread_progress(self, thread_id, progress, thread_name):
        """Update the progress bar for a specific thread."""
        if thread_id < len(self.progress_bars):
            self.progress_bars[thread_id].setValue(progress)
            self.thread_labels[thread_id].setText(f"Thread nr. {thread_name}")
            self.percent_labels[thread_id].setText(f"{progress} %")

    def clear_progress_bars(self):
        """Reset all progress bars."""
        for bar in self.progress_bars:
            bar.setValue(0)

# Main Application Window
class SelectorApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.base_dir = None #Default base directory
        self.config_file = None  # Initial config file
        #self.reader = ReplayConfReader(self.config_file, self.base_dir)  # Initialize with the default config
        #self.file_data = self.reader.find_matching_files()  # Get file data based on the initial config
        self.file_data = {}
        self.initUI()
        self.progress_window = None  # Initialize progress window as None

    def initUI(self):

        self.setWindowTitle("AGATA and ancillary selector @ LNL")
        # Create the central widget
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)

        # Set fixed window size (optional)
        self.setFixedSize(1000, 1000)  # Example fixed size

        # Initialize the layout
        self.layout = QVBoxLayout()


        self.initialization = QHBoxLayout()
        
        # Adding two logos aligned in height
        self.logo_label1 = QLabel(self)
        self.logo_label2 = QLabel(self)

        # Load the first logo
        self.logo_pixmap2 = QPixmap("agata_logo.png")  # Replace with your first logo path
        # Load the second logo
        self.logo_pixmap1 = QPixmap("Logo_INFN.png")  # Replace with your second logo path

        # Rescale the pixmaps to your desired size (e.g., 100x100)
        scaled_pixmap2 = self.logo_pixmap2.scaled(40, 40, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        scaled_pixmap1 = self.logo_pixmap1.scaled(60, 60, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        
        # Set the scaled pixmaps on the labels
        self.logo_label1.setPixmap(scaled_pixmap1)
        self.logo_label2.setPixmap(scaled_pixmap2)

        # Align the labels to the bottom-right of the window
        self.logo_label1.setAlignment(Qt.AlignRight)
        self.logo_label2.setAlignment(Qt.AlignRight)

        # Create a layout to hold both logos horizontally
        logo_layout = QHBoxLayout()
        logo_layout.addWidget(self.logo_label1)
        logo_layout.addWidget(self.logo_label2)

        # Optionally adjust spacing between logos
        logo_layout.setSpacing(5)  # Adjust as needed

        # Add the logo layout to the main layout
        logo_widget = QWidget(self)
        logo_widget.setLayout(logo_layout)
        self.layout.addWidget(logo_widget, alignment=Qt.AlignRight)

         # Initialize the label to show the selected directory
        self.conf_dir = QVBoxLayout()
        self.dir_label = QLabel(f"Selected Directory: {self.base_dir}", self)
        self.conf_dir.addWidget(self.dir_label)

        # Add a button to browse for the directory
        self.browse_button = QPushButton("Browse for Analysis Directory", self)
        self.browse_button.clicked.connect(self.browse_directory)
        self.conf_dir.addWidget(self.browse_button)

        # Add file selection for the selector.conf using QComboBox
        self.conf_file = QVBoxLayout()
        self.config_label = QLabel("Select .conf file:", self)
        self.conf_file.addWidget(self.config_label)

        # Config File entry with dropdown
        self.config_combobox = QComboBox(self)
        self.conf_file.addWidget(self.config_combobox)

        self.initialization.addLayout(self.conf_dir)
        self.initialization.addLayout(self.conf_file)
        self.layout.addLayout(self.initialization)

        # Initially populate the combobox with .conf files
        self.populate_config_combobox()
        self.config_combobox.currentTextChanged.connect(self.update_selected_config)
        #self.layout.addWidget(self.config_combobox)

        # Display the selected file path
        self.load_refresh = QHBoxLayout()

        self.refresh_button = QPushButton("Refresh", self)
        self.refresh_button.clicked.connect(self.update_selected_config)
        self.refresh_button.setFixedWidth(70)
        self.load_refresh.addWidget(self.refresh_button)

        self.selected_config_path_label = QLabel("No file selected", self)
        self.selected_config_path_label.setMaximumWidth(1000)  # Avoid expansion
        self.selected_config_path_label.setWordWrap(True)
        self.load_refresh.addWidget(self.selected_config_path_label)

        self.layout.addLayout(self.load_refresh)

        # Dynamic progress bars for each thread (added here)
        self.progress_bars = []

        self.table_checkboxes = QHBoxLayout()
        self.table = QTableWidget(len(self.file_data), 6)  # 6 columns
        self.table.setHorizontalHeaderLabels([
            'Process', 'Run Number', 'Input Files', 'Output Files', 'Sum File', 'Open With'
        ])


        self.update_table()  # Populate the table with data
        self.setup_table()   # Add dynamic "Open With" logic

        self.table_checkboxes.addWidget(self.table)
        self.layout.addLayout(self.table_checkboxes)

        # Add other options as checkboxes
        self.checkboxes = QVBoxLayout()
        self.no_user_sel_checkbox = QCheckBox("No User Selector", self)
        self.checkboxes.addWidget(self.no_user_sel_checkbox)
        self.rm_partial_checkbox = QCheckBox("Remove Partial Files", self)
        self.checkboxes.addWidget(self.rm_partial_checkbox)
        self.distribute_checkbox = QCheckBox("Distribute Work", self)
        self.checkboxes.addWidget(self.distribute_checkbox)
        self.optimize_checkbox = QCheckBox("Run Optimizer", self)
        self.checkboxes.addWidget(self.optimize_checkbox)
        self.only_enabled_histos_checkbox = QCheckBox("Only Enabled Histos", self)
        self.checkboxes.addWidget(self.only_enabled_histos_checkbox)
        self.debug_canvas_checkbox = QCheckBox("Debug Canvas", self)
        self.checkboxes.addWidget(self.debug_canvas_checkbox)
        self.mem_check_checkbox = QCheckBox("Memory Check", self)
        self.checkboxes.addWidget(self.mem_check_checkbox)
        self.no_hadd_checkbox = QCheckBox("No Hadd", self)
        self.checkboxes.addWidget(self.no_hadd_checkbox)
        self.update_runs_checkbox = QCheckBox("Update Runs", self)
        self.checkboxes.addWidget(self.update_runs_checkbox)
        self.update_prisma_checkbox = QCheckBox("Update Prisma", self)
        self.checkboxes.addWidget(self.update_prisma_checkbox)
        self.sum_all_checkbox = QCheckBox("Sum All Runs", self)
        self.checkboxes.addWidget(self.sum_all_checkbox)
        self.subtract_bkg_checkbox = QCheckBox("Subtract Background", self)
        self.checkboxes.addWidget(self.subtract_bkg_checkbox)
        self.table_checkboxes.addLayout(self.checkboxes)

        #self.layout.addLayout(self.table_checkboxes)

        self.options = QHBoxLayout()

        self.th_evts_verb = QHBoxLayout()
        # Add the threads input
        self.threads_label = QLabel("Number of Threads:", self)
        self.th_evts_verb.addWidget(self.threads_label)
        self.threads_input = QSpinBox(self)
        self.threads_input.setValue(6)
        self.th_evts_verb.addWidget(self.threads_input)

        # Add the verbosity label and input (replacing QComboBox with QLineEdit)
        self.verbosity_label = QLabel("Verbosity (0 to get progress):", self)
        self.th_evts_verb.addWidget(self.verbosity_label)
        self.verbosity_input = QSpinBox(self)
        self.th_evts_verb.addWidget(self.verbosity_input)
        self.layout.addLayout(self.th_evts_verb)

        # Add the events input
        self.events_label = QLabel("Number of Events:", self)
        self.th_evts_verb.addWidget(self.events_label)
        self.events_input = QLineEdit(self)
        self.th_evts_verb.addWidget(self.events_input)

        # Add the iofile input
        self.iofile_layout = QHBoxLayout()
        self.iofile_label = QLabel("Enter Input File:", self)
        self.iofile_layout.addWidget(self.iofile_label)
        self.iofile_input = QLineEdit(self)  # This is the missing widget
        self.iofile_layout.addWidget(self.iofile_input)
        
        # Add UI elements for input/output list file
        self.iolistfile_label = QLabel("Enter Output File:", self)  # New label
        self.iofile_layout.addWidget(self.iolistfile_label)
        self.iolistfile_input = QLineEdit(self)  # This is the iolistfile input widget
        self.iofile_layout.addWidget(self.iolistfile_input)
        self.layout.addLayout(self.iofile_layout)

         # Add UI elements for set gates file
        self.set_gates_label = QLabel("Enter Set Gates File:", self)  # New label
        self.layout.addWidget(self.set_gates_label)
        self.set_gates_input = QLineEdit(self)  # This is the set gates file input widget
        self.layout.addWidget(self.set_gates_input)


        # Add window size input
        self.window_size_label = QLabel("Window Size:", self)
        self.layout.addWidget(self.window_size_label)
        self.window_size_input = QLineEdit(self)
        self.layout.addWidget(self.window_size_input)

        # Add the run button
        self.start_kill = QHBoxLayout()
        self.run_button = QPushButton('Start Selector', self)
        self.run_button.clicked.connect(self.start_selector)
        self.start_kill.addWidget(self.run_button)
        
        # Add the kill button
        self.stop_button = QPushButton('Stop Selector', self)
        self.stop_button.clicked.connect(self.stop_selector)
        self.stop_button.setEnabled(False)  # Initially disabled
        self.start_kill.addWidget(self.stop_button)
        self.layout.addLayout(self.start_kill)

        # Add the kill button
        self.kill_button = QPushButton('Kill Selector', self)
        self.kill_button.clicked.connect(self.kill_selector)
        self.kill_button.setEnabled(False)  # Initially disabled
        self.start_kill.addWidget(self.kill_button)
        self.layout.addLayout(self.start_kill)
        
        # Add a QTextEdit widget for output display
        self.output_title_label = QLabel("Output Display:", self)
        self.layout.addWidget(self.output_title_label)
        self.output_display = QTextEdit(self)  # This is the output display widget
        self.output_display.setReadOnly(True)  # Set it to read-only
        self.layout.addWidget(self.output_display)

        # Set the layout to the central widget
        self.layout.addLayout(self.options)
        central_widget.setLayout(self.layout)
    
    def browse_directory(self):
        # Open the file dialog for selecting a directory
        selected_dir = QFileDialog.getExistingDirectory(self, "Select Directory", self.base_dir)

        if selected_dir:
            self.base_dir = selected_dir  # Update base_dir with the selected path
            self.dir_label.setText(f"Selected Directory: {self.base_dir}")  # Update the label

            # Repopulate the config_combobox with files from the new base_dir
            self.populate_config_combobox()
        else:
            self.dir_label.setText("No directory selected")  # Update the label

    def populate_config_combobox(self):
        """Populate the config_combobox with .conf files from the base directory."""
        self.config_combobox.clear()  # Clear any existing items
        self.config_combobox.addItem("Select a config")  # Add default option

        # Loop through the directory and add .conf files to the combobox
        for file in os.listdir(self.base_dir):
            if file.endswith(".conf"):
                self.config_combobox.addItem(file)
    
    def update_data(self):
        if self.base_dir and self.conf_file:
            print(f"Looking for {self.config_file} in dir {self.base_dir}")
            self.reader = ReplayConfReader(self.config_file, self.base_dir)
            # Re-populate file_data based on the selected config file
            self.file_data = self.reader.find_matching_files()
            # Update the table after changing the config file
            self.update_table()

    def update_selected_config(self, selected_config):
        """Update the label and internal state when a config is selected."""
        if selected_config == "Select a config" or selected_config == '' or not selected_config or not self.base_dir:
            self.selected_config_path_label.setText("No file selected")
            self.config_file = None
        else:
            self.selected_config_path_label.setText(f"Selected: {selected_config}")
            self.config_file = os.path.join(self.base_dir, selected_config)
            self.update_data()

    def update_table(self):
        """Update the table with the data from the current file_data."""
        self.table.clearContents()
        self.table.setRowCount(len(self.file_data))  # Adjust number of rows to match the data
        self.table.setColumnWidth(5, 250)
        self.table.setColumnWidth(4, 150)


        for row, (run, data) in enumerate(self.file_data.items()):
            self.table.setRowHeight(row, 50)
            # Process column with checkbox
            process_checkbox = QCheckBox()
            self.table.setCellWidget(row, 0, process_checkbox)

            # Run number column
            self.table.setItem(row, 1, QTableWidgetItem(run))

            # Input Files column
            self.table.setItem(row, 2, QTableWidgetItem(str(data['input_files_count'])))

            # Output Files column
            self.table.setItem(row, 3, QTableWidgetItem(str(data['output_files_count'])))

            # Sum File column
            sum_file_text = os.path.basename(data['sum_file_name']) if data['sum_file_present'] else 'No'
            self.table.setItem(row, 4, QTableWidgetItem(sum_file_text))
        
        # Add the new column for "Open With"
        self.setup_table()

    def setup_table(self):
        for row in range(self.table.rowCount()):
            row_data = self.file_data[list(self.file_data.keys())[row]]
            sum_file_name = row_data.get('sum_file_name', None)

            if sum_file_name:
                open_layout = QHBoxLayout()
                cell_widget = QWidget()
                cell_widget.setLayout(open_layout)

                combo_box = QComboBox()
                combo_box.addItems(["ROOT TBrowser", "Cubix"])
                open_layout.addWidget(combo_box)

                open_button = QPushButton("Open", self)
                open_button.clicked.connect(
                    partial(self.handle_open_command_from_button, combo_box, sum_file_name)
                )
                open_layout.addWidget(open_button)

                self.table.setCellWidget(row, 5, cell_widget)
            else:
                combo_box = QComboBox()
                combo_box.addItems(["ROOT TBrowser", "Cubix"])
                combo_box.setEnabled(False)
                self.table.setCellWidget(row, 5, combo_box)

    def handle_open_command_from_button(self, combo_box, file_name):
        index = combo_box.currentIndex()
        self.handle_open_command(index, file_name)
    
    def handle_open_command(self, index, file_name):
        command = f"{'rootbrowse' if index == 0 else 'cubix'} {os.path.join(self.base_dir, file_name)}"
        def run_subprocess():
            try:
                subprocess.run(command, shell=True, check=True)
                self.output_display.append(f"Executed: {command}")
            except subprocess.CalledProcessError as e:
                self.output_display.append(f"Error executing command: {command}\n{e}")

         # Start a new thread to run the subprocess
        thread = threading.Thread(target=run_subprocess)
        thread.start()


    def on_selector_finished(self):
        # Re-enable the run button
        self.run_button.setDisabled(False)

    def start_selector(self):
        
        # Collect selected run numbers from the table
        selected_runs = []
        for row in range(self.table.rowCount()):
            checkbox = self.table.cellWidget(row, 0)  # Checkbox in column 0
            if checkbox.isChecked():  # Check if the checkbox is checked
                run_number_item = self.table.item(row, 1)  # Run Number column (index 1)
                if run_number_item:  # Ensure the item exists
                    run_number = run_number_item.text()
                
                    ## Get the selected option from the new column
                    #combo_box = self.table.cellWidget(row, 5)  # New column index is 6
                    #if combo_box:
                    #    open_with = combo_box.currentText()  # Get the selected value ("TBrowser" or "Cubix")

                    ## Add the run number and open_with option to the selected runs
                    #selected_runs.append((run_number, open_with))
                    selected_runs.append(run_number)
                    ##selected_runs.append(run_number_item.text())  # Add the text of the run number
        # Process the selected_runs as needed
        #for run, open_with in selected_runs:
        #    print(f"Run: {run}, Open With: {open_with}")

        # Set the default number of threads based on the number of selected runs
        num_threads = len(selected_runs)

        # If no runs are selected, set threads to 0
        if num_threads == 0:
            num_threads = 6
        
        # Get the number of threads from the input field
        num_threads_input = self.threads_input.value()  # Use value() for QSpinBox

        # Validate user input
        if num_threads_input > 0:
            num_threads = num_threads_input
        
        # Update the spin box to reflect the final number of threads
        self.threads_input.setValue(num_threads)  # Use setValue() for QSpinBox

        # Create and show progress window
        if not self.progress_window or not self.progress_window.isVisible():
            self.progress_window = ProgressWindow(num_threads)
            self.progress_window.show()


        # Use selected runs as input for the command
        run_numbers = selected_runs
      
        # Get other inputs
        num_events = self.events_input.text().strip()
        window_size = self.window_size_input.text().strip()

        # Get the verbosity value from the spin box
        verbosity = self.verbosity_input.value()  # Use value() for QSpinBox

        # Add verbosity option only if verbosity >= 1
        if verbosity > 0:
            verbosity_option = f"--verb={verbosity}"  # Correct flag for verbosity
        else:
            verbosity_option = None  # Skip verbosity if 0

        # Construct the command
        if SelectorRunner.is_executable("RunSelector"):
            command = ["RunSelector"]
        elif self.base_dir == None:
            return
        elif SelectorRunner.is_executable(os.path.join(self.base_dir, "RunSelector")):
            command = [os.path.join(self.base_dir, "RunSelector")]
        else:
            return

        command += run_numbers

        # Add --nrthr (number of threads)
        command += ["--nrthr", str(num_threads)]

        # Add --nrevts (number of events)
        if num_events.isdigit():
            command += ["--nrevts", num_events]

        # Add verbosity option (if provided)
        if verbosity_option:
            command += [verbosity_option]

        # Optionally add window size
        if window_size.isdigit():
            command += ["--windowsize", window_size]
        
        # Handle --conf (config file)
        if self.config_file:
            command += ["--conf", self.config_file]

        # Handle --iofile (input and output files)
        io_files = self.iofile_input.text().strip()
        if io_files:
            command += ["--iofile", io_files]

        # Add input/output list file if provided
        iolistfile = self.iolistfile_input.text().strip()  # This is the input/output list file
        if iolistfile:
            command += ["--iolistfile", iolistfile]

        # Handle --set_gates (selector file)
        set_gates = self.set_gates_input.text().strip()
        if set_gates:
            command += ["--set_gates", set_gates]

        # Handle various checkboxes/options
        if self.no_user_sel_checkbox.isChecked():
            command.append("--no_user_sel")
        if self.rm_partial_checkbox.isChecked():
            command.append("--rm_partial")
        if self.distribute_checkbox.isChecked():
            command.append("--distribute")
        if self.optimize_checkbox.isChecked():
            command.append("--optimize")
        if self.only_enabled_histos_checkbox.isChecked():
            command.append("--only_enabled_histos")
        if self.debug_canvas_checkbox.isChecked():
            command.append("--debug_canvas")
        if self.mem_check_checkbox.isChecked():
            command.append("--mem_check")
        if self.no_hadd_checkbox.isChecked():
            command.append("--no_hadd")
        if self.update_runs_checkbox.isChecked():
            command.append("--update_runs")
        if self.update_prisma_checkbox.isChecked():
            command.append("--update_prisma")
        if self.sum_all_checkbox.isChecked():
            command.append("--sum_all")
        if self.subtract_bkg_checkbox.isChecked():
            command.append("--subtract_bkg")

        # Start the SelectorRunner thread
        self.run_button.setDisabled(True)
        self.runner = SelectorRunner(command, num_threads, len(run_numbers), self.base_dir)
        self.runner.progress.connect(self.update_output)
        self.runner.thread_progress.connect(self.progress_window.update_thread_progress)
        self.runner.clear_progress_bars.connect(self.progress_window.clear_progress_bars)
        self.runner.finished.connect(self.on_selector_finished)
        self.runner.start()
        self.kill_button.setEnabled(True)  # Enable the kill button
        self.stop_button.setEnabled(True)  # Enable the kill button


    def kill_selector(self):
        """Stop the running SelectorRunner."""
        if self.runner:
            self.runner.kill()
            self.kill_button.setEnabled(False)  # Disable the kill button
            self.stop_button.setEnabled(False)  # Disable the kill button
            self.update_output("Selector process has been terminated.")

    def stop_selector(self):
        if self.runner:
            self.runner.stop(self.base_dir)
            self.kill_button.setEnabled(False)  # Disable the kill button
            self.stop_button.setEnabled(False)  # Disable the kill button
            self.update_output("Selector process has been stopped.")
            

    def update_output(self, message):
        self.output_display.append(message)


    def process_selected_runs(self):
        selected_runs = []
        for row in range(self.table.rowCount()):
            checkbox = self.table.cellWidget(row, 0)
            if checkbox.isChecked():
                run = self.table.item(row, 1).text()
                config_file = self.table.cellWidget(row, 5).currentText()
                selected_runs.append((run, config_file))

        # Print or process selected runs here
        print("Selected runs and config files:")
        for run, config in selected_runs:
            print(f"Run: {run}, Config File: {config}")

            
# Main Entry Point
if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = SelectorApp()
    window.show()
    sys.exit(app.exec_())

