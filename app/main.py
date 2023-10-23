import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
import os
import csv
from itertools import zip_longest
import pandas as pd
import glob

def browse_group_file():
    file_path = filedialog.askopenfilename(title="Select Group.csv file", filetypes=[("CSV Files", "*.csv")])
    group_file_entry.delete(0, tk.END)  # Clear any previous entry
    group_file_entry.insert(tk.END, file_path)

def browse_date_files():
    file_paths = filedialog.askopenfilenames(title="Select date CSV files", filetypes=[("CSV Files", "*.csv")])
    date_files_entry.delete(0, tk.END)  # Clear any previous entries
    for file_path in file_paths:
        date_files_entry.insert(tk.END, file_path)

def browse_output_directory():
    directory_path = filedialog.askdirectory(title="Select an output directory")
    output_directory_entry.delete(0, tk.END)  # Clear any previous entry
    output_directory_entry.insert(tk.END, directory_path)

# Function to process a single <date>.csv file and extract names
def process_date_csv(file, identifiers):
    '''Return list of true, false, or possible hits for each person in Group.csv and missed names at the end.'''
    df = pd.read_csv(file, header=2)  # Skip the first 3 rows
    date = os.path.splitext(os.path.basename(file))[0]
    # Create a list to store the rows for this date
    date_entries = [date] + (['False'] * len(identifiers))
    misses = ['', 'Unknown names from Zoom:']
    print(df)
    # Process each row in the <date>.csv file
    for _, row in df.iterrows():
        full_name = row['Name (Original Name)']
        name = full_name.split()
        is_match = False
        # For each row in Group.csv, check if person can be identified in Zoom .csv log
        for i, identifier in enumerate(identifiers):
            if identifier["first_name"] == 'NaN' or date_entries[i + 1] == 'True':
                is_match = True
            # full match: name contains unique identifier
            elif (identifier["first_is_unique"] and identifier["first_name"] in name) or \
                (identifier["last_is_unique"] and identifier["last_name"] in name) or \
                    (identifier["first_name"] in name and identifier["last_name"] in name):
                        date_entries[i + 1] = 'True'
                        is_match = True
            elif identifier["first_name"] in name or identifier["last_name"] in name:
                if date_entries[i + 1] == 'False':
                    date_entries[i + 1] = f'possible match: {full_name}'
                    is_match = True
                else:
                    date_entries[i + 1] += ", " + full_name
                    is_match = True
        if not is_match:
            misses += [f'{full_name}']
                
    date_entries += misses
    return date_entries

def process_files():
    group_file_path = group_file_entry.get()
    date_file_paths = date_files_entry.get().split()
    output_file_path = output_directory_entry.get()

    if not group_file_path or not date_file_paths or not output_file_path:
        messagebox.showerror("Error", "Please select Group.csv, date CSV files, and an output file.")
        return

    # Input and output directories
    # TODO: input_directory = 'testData'  Folder containing <date>.csv files
    # TODO: output_directory = 'outputs'  Folder where output files will be stored

    # Create the output directory if it doesn't exist
    os.makedirs(output_file_path, exist_ok=True)

    # Read the Group.csv file and create a dictionary of identifiers
    # TODO: group_file = os.path.join(input_directory, 'Group.csv')
    group_data = pd.read_csv(group_file_path)
    print(group_data) #debug
    identifiers = []
    index = {}

    for _, group_row in group_data.iterrows():
        first_name = str(group_row['First Name']).rstrip()
        last_name = str(group_row['Last Name']).rstrip()
        if first_name in index:
            index[first_name] += 1
        else:
            index[first_name] = 1
        if last_name in index:
            index[last_name] += 1
        else:
            index[last_name] = 1

    for _, group_row in group_data.iterrows():
        first_name = str(group_row['First Name']).rstrip()
        last_name = str(group_row['Last Name']).rstrip()
        first_is_unique = index[first_name] == 1
        last_is_unique = index[last_name] == 1
        identifiers += [
            {"first_name": first_name,
            "last_name": last_name,
            "first_is_unique": first_is_unique,
            "last_is_unique": last_is_unique}]

    print(index) #debug
    print(identifiers) #debug

    # Process each <date>.csv file in the input directory
    # TODO: date_csv_files = glob.glob(os.path.join(input_directory, 'testOct*.csv'))
    date_cols = []
    for date_csv_file in date_file_paths:
        # Process the <date>.csv file and get the cols for the output CSV
        date_cols.append(process_date_csv(date_csv_file, identifiers))

    # Transpose cols to use writerows
    print(date_cols)
    date_rows = list(zip_longest(*date_cols, fillvalue=''))

    # Write the output CSV file to the output directory    
    output_file = os.path.join(output_file_path, "output_test.tsv")
    with open(output_file, mode='w') as f:
        writer = csv.writer(f, delimiter='\t')
        # for row in date_rows:
            
        #     if (type )
        #     f.write(row + "\n")
        for row in date_rows:
            formatted_row = []
            # breakpoint()
            for cell in row:
                if isinstance(cell, list):
                    formatted_row.append(", ".join(map(str, cell)))
                else:
                    formatted_row.append(cell)
            writer.writerow(formatted_row)
    print(f'Output CSV file created: {output_file_path}')

    messagebox.showinfo("Info", "Files processed successfully.")

# Create the main window
root = tk.Tk()
root.title("CSV File Processor")

# Create and configure Group.csv input widgets
group_file_label = tk.Label(root, text="Select Group.csv file:")
group_file_label.pack()
group_file_entry = tk.Entry(root, width=40)
group_file_entry.pack()
browse_group_button = tk.Button(root, text="Browse", command=browse_group_file)
browse_group_button.pack()

# Create and configure date.csv files input widgets
date_files_label = tk.Label(root, text="Select date CSV files:")
date_files_label.pack()
date_files_entry = tk.Entry(root, width=40)
date_files_entry.pack()
browse_date_button = tk.Button(root, text="Browse", command=browse_date_files)
browse_date_button.pack()

# Create and configure output directory input widgets
output_directory_label = tk.Label(root, text="Select the output directory:")
output_directory_label.pack()
output_directory_entry = tk.Entry(root, width=40)
output_directory_entry.pack()
browse_output_directory_button = tk.Button(root, text="Browse", command=browse_output_directory)
browse_output_directory_button.pack()

# Create and configure process button
process_button = tk.Button(root, text="Process Files", command=process_files)
process_button.pack()

root.mainloop()