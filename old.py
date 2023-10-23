import csv
from itertools import zip_longest
import os
import pandas as pd
import glob

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


# Input and output directories
input_directory = 'testData'  # Folder containing <date>.csv files
output_directory = 'outputs'  # Folder where output files will be stored

# Create the output directory if it doesn't exist
os.makedirs(output_directory, exist_ok=True)

# Read the Group.csv file and create a dictionary of identifiers
group_file = os.path.join(input_directory, 'Group.csv')
group_data = pd.read_csv(group_file)
print(group_data) #debug
identifiers = []
index = {}

for row_num, group_row in group_data.iterrows():
    first_name = group_row['First Name']
    last_name = group_row['Last Name']
    if first_name in index:
        index[first_name] += 1
    else:
        index[first_name] = 1
    if last_name in index:
        index[last_name] += 1
    else:
        index[last_name] = 1
for row_num, group_row in group_data.iterrows():
    first_name = group_row['First Name']
    last_name = group_row['Last Name']
    first_is_unique = index[first_name] == 1
    last_is_unique = index[last_name] == 1
    identifiers += [
        {"first_name": str(first_name),
        "last_name": str(last_name),
        "first_is_unique": first_is_unique,
        "last_is_unique": last_is_unique}]

print(index) #debug
print(identifiers) #debug

# Process each <date>.csv file in the input directory
date_csv_files = glob.glob(os.path.join(input_directory, 'testOct*.csv'))
date_cols = []
for date_csv_file in date_csv_files:
    # Process the <date>.csv file and get the cols for the output CSV
    date_cols.append(process_date_csv(date_csv_file, identifiers))

# Transpose cols to use writerows
print(date_cols)
date_rows = list(zip_longest(*date_cols, fillvalue=''))

# Write the output CSV file to the output directory    
output_file = os.path.join(output_directory, "output_test.tsv")
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
print(f'Output CSV file created: {output_file}')
