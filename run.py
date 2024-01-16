import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
import os
import csv
from itertools import zip_longest
import pandas as pd
# import glob

def is_gl(zoom_name):
    gl_list = ["UM", "GIDAS", "GL"]
    return (any(gl_id in zoom_name.split() for gl_id in gl_list))

def browse_group_file():
    file_path = filedialog.askopenfilename(title="Select Group.csv file", filetypes=[("CSV Files", "*.csv")])
    group_file_entry.delete(0, tk.END)  # Clear any previous entry
    group_file_entry.insert(tk.END, file_path)

def browse_date_files():
    file_paths = filedialog.askopenfilenames(title="Select date CSV files", filetypes=[("CSV Files", "*.csv")])
    date_files_entry.delete(0, tk.END)  # Clear any previous entries
    for file_path in file_paths:
        date_files_entry.insert(tk.END, file_path + ' ')

def browse_output_directory():
    directory_path = filedialog.askdirectory(title="Select an output directory")
    output_directory_entry.delete(0, tk.END)  # Clear any previous entry
    output_directory_entry.insert(tk.END, directory_path)

def toggle_dev():
    if dev_mode_entry.get():
        # Set default values for the browse fields
        group_file_entry.delete(0, tk.END)
        group_file_entry.insert(tk.END, "data/Group.csv")
        date_files_entry.delete(0, tk.END)
        date_files_entry.insert(tk.END, "data/Oct14.csv")
        output_directory_entry.delete(0, tk.END)
        output_directory_entry.insert(tk.END, "dev_output")
        output_filename_entry.delete(0, tk.END)
        output_filename_entry.insert(tk.END, "dev_output.tsv")
        
def matches(name_queries, zoom_name):
    """Find if any element in word in string A is in list B, ignoring parentheses"""
    return any(temp in zoom_name.replace('(', ' ').replace(')', ' ').lower().split() \
        for temp in name_queries.replace('(', ' ').replace(')', ' ').lower().split())
    
# Function to process a single <date>.csv file and extract names
def process_date_csv(file, identifiers, is_dev_mode, include_gls, min_time):
    '''Return list of true, false, or possible hits for each person in Group.csv and missed names at the end.'''
    # get date of this file
    # date = os.path.splitext(os.path.basename(file))[0]
    date_df = pd.read_csv(file)
    date = pd.to_datetime(date_df['Start Time'][0]).strftime('%m/%d/%Y')
    
    df = pd.read_csv(file, header=2)  # Skip the first 3 rows
    
    # Create a list to store the rows for this date
    date_entries = [date] + (['False'] * len(identifiers))
    misses = ['', 'Unmatched Zoom names:']
    # print(df) #debug
    
    # Process each row in the <date>.csv file
    for _, row in df.iterrows():
        zoom_name = row['Name (Original Name)']
        duration = row['Total Duration (Minutes)']
        is_match = False
        
        # For each row in Group.csv, check if person can be identified in Zoom .csv log
        for i, identifier in enumerate(identifiers):
            if not include_gls and is_gl(zoom_name): 
                is_match = True # skips the rest
                break
            # split first/last name and check if they exist in this row of the Zoom log
            first_matches = matches(identifier["first_name"], zoom_name)
            last_matches = matches(identifier["last_name"], zoom_name)
            # check for blank lines or if it is already matched
            # if identifier["first_name"] == 'NaN' or date_entries[i + 1] == 'True':
            #     is_match = True
            # full match: name contains unique identifier
            if (identifier["first_is_unique"] and first_matches) or \
                (identifier["last_is_unique"] and last_matches) or \
                    (first_matches and last_matches):
                        if min_time is not None and duration < min_time:
                            continue
                            # skip full match if they are not in the meeting long enough
                        date_entries[i + 1] = 'True'
                        if is_dev_mode:
                            date_entries[i + 1] += f': {zoom_name} ({duration})'
                        is_match = True
        if not is_match: 
            # check for partial matches
            for i, identifier in enumerate(identifiers):
                if (not include_gls and is_gl(zoom_name)) or date_entries[i + 1] == 'True': 
                    # don't check for partial matches for this person in Group.csv
                    break
                # split first/last name and check if they exist in this row of the Zoom log
                first_matches = matches(identifier["first_name"], zoom_name)
                last_matches = matches(identifier["last_name"], zoom_name)                
                # partial match: name contains non-unique first or last name
                if first_matches or last_matches:
                    is_match = True
                    if date_entries[i + 1] == 'False':
                        # replace default 'False' with partial match, don't overwrite true
                        date_entries[i + 1] = f'Partial: {zoom_name} ({duration})'
                    elif is_dev_mode:
                        # specify it is a partial match
                        date_entries[i + 1] += f', Partial: {zoom_name} ({duration})'
                    else:
                        # append to save space
                        date_entries[i + 1] += f', {zoom_name} ({duration})'
        if not is_match:
            misses += [f'{zoom_name}']

    date_entries += misses
    return date_entries

def process_files():
    is_dev_mode = dev_mode_entry.get()
    group_file_path = group_file_entry.get()
    date_file_paths = date_files_entry.get().split()
    output_file_path = output_directory_entry.get()
    output_filename = output_filename_entry.get()
    minimum_time = minimum_time_entry.get()
    try:
        minimum_time = int(minimum_time)
    except ValueError:
        messagebox.showerror("Error", "Please enter a valid number for the minimum time")
        return
    include_gls = include_gls_entry.get()

    if not group_file_path or not date_file_paths or not output_file_path:
        messagebox.showerror("Error", "Please select Group.csv, date CSV files, and an output file.")
        return

    # Create the output directory if it doesn't exist
    os.makedirs(output_file_path, exist_ok=True)

    # Read the Group.csv file and create a dictionary of identifiers
    group_data = pd.read_csv(group_file_path)
    # print(group_data) #debug
    identifiers = []
    index = {}

    for _, group_row in group_data.iterrows():
        first_name = str(group_row['First Name']).rstrip()
        last_name = str(group_row['Last Name']).rstrip()
        leader_full_name = str(group_row['Leader']).rstrip()
        if first_name in index:
            index[first_name] += 1
        else:
            index[first_name] = 1
        if last_name in index:
            index[last_name] += 1
        else:
            index[last_name] = 1
        for split_leader_name in leader_full_name.split():
            if split_leader_name in index:
                index[split_leader_name] += 1
            else:
                index[split_leader_name] = 1

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

    # print(index) #debug
    # print(identifiers) #debug

    # Process each <date>.csv file in the input directory
    date_cols = []
    for date_csv_file in date_file_paths:
        # Process the <date>.csv file and get the cols for the output CSV
        date_cols.append(process_date_csv(date_csv_file, identifiers, is_dev_mode, include_gls, minimum_time))

    # Transpose cols to use writerows
    print(date_cols)
    # sort by first element 
    date_cols = sorted(date_cols, key=lambda x: x[0])
    date_rows = list(zip_longest(*date_cols, fillvalue=''))

    # Write the output CSV file to the output directory    
    output_file = os.path.join(output_file_path, output_filename)
    with open(output_file, mode='w') as f:
        writer = csv.writer(f, delimiter='\t')
        
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
root.title("miRcore Attendance Logger")

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

# Create a label to describe the text entry
label = tk.Label(root, text="Enter filename for output:")
label.pack()
output_filename_entry = tk.Entry(root)
output_filename_entry.insert(0, "output.tsv")  # Set the default value
output_filename_entry.pack()

# Create a label and entry for the minimum_time
minimum_time_label = tk.Label(root, text="Minimum time (minutes) in Zoom to be marked as full match:")
minimum_time_label.pack()
minimum_time_entry = tk.Entry(root)
minimum_time_entry.pack()
minimum_time_entry.insert(0, "0")  # Set the default value

# Create a BooleanVar to control the include_gl mode
include_gls_entry = tk.BooleanVar()
include_gls_entry.set(False)  # Set the initial state to False

# Create a checkbox for the include_gl mode
gl_checkbox = tk.Checkbutton(root, text="Include GLs", variable=include_gls_entry)
gl_checkbox.pack()

# Create a BooleanVar to control the dev mode
dev_mode_entry = tk.BooleanVar()
dev_mode_entry.set(False)  # Set the initial state to False

# Create a checkbox for the dev mode
dev_checkbox = tk.Checkbutton(root, text="Developer Mode", variable=dev_mode_entry, command=toggle_dev)
dev_checkbox.pack()

# Create and configure process button
process_button = tk.Button(root, text="Process Files", command=process_files)
process_button.pack()

root.mainloop()