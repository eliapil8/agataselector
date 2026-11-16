import sys
import os
from run_manager import ReplayConfReader
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QWidget, QTableWidget, QTableWidgetItem, QCheckBox, QComboBox, QVBoxLayout, QHBoxLayout, QPushButton, QFileDialog


class ReplayApp(QWidget):
    def __init__(self):
        super().__init__()
        self.base_dir = "../build"
        self.reader = ReplayConfReader("selector.conf", self.base_dir)
        self.file_data = self.reader.find_matching_files()
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle('Run Configuration')
        layout = QVBoxLayout()

        # Create a table with the appropriate number of columns
        
        self.table = QTableWidget(len(self.file_data), 6)  # 6 columns
        self.table.setHorizontalHeaderLabels([
            'Process', 'Run Number', 'Input Files', 'Output Files', 'Sum File', 'Config File'
        ])

        # Populate the table with data
        for row, (run, data) in enumerate(self.file_data.items()):
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
            sum_file_text = 'Yes' if data['sum_file_present'] else 'No'
            self.table.setItem(row, 4, QTableWidgetItem(sum_file_text))

            # Config File column with dropdown
            config_combobox = QComboBox()
            config_combobox.addItem('Select a config')
            # Load config files from base directory (just an example, adjust as necessary)
            for file in os.listdir(self.base_dir):
                if file.endswith('.conf'):
                    config_combobox.addItem(file)
            self.table.setCellWidget(row, 5, config_combobox)

        layout.addWidget(self.table)

        # Add a button to process selected rows
        process_button = QPushButton('Process Selected Runs')
        process_button.clicked.connect(self.process_selected_runs)
        layout.addWidget(process_button)

        self.setLayout(layout)

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


if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = ReplayApp()
    window.resize(800, 400)
    window.show()

    sys.exit(app.exec_())