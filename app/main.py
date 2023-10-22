import os
import pandas as pd
import glob

# Function to process a single <date>.csv file and extract names
def process_date_csv(file, identifiers):
    df = pd.read_csv(file, skiprows=3)  # Skip the first 3 rows
    date = os.path.splitext(os.path.basename(file))[0]

    # Create a list to store the rows for this date
    date_rows = []

    # Process each row in the <date>.csv file
    for index, row in df.iterrows():
        name = row['Name (Original Name)']
        matched = False
        partial_matches = []

        # Check if the name matches any unique identifiers
        for identifier, group_row in identifiers.items():
            if name == identifier:
                matched = True
                break
            if name in group_row:
                partial_matches.append(name)

        if matched:
            date_rows.append('true')
        elif partial_matches:
            date_rows.append(", ".join(partial_matches))
        else:
            date_rows.append('false')

    return date_rows

# Input and output directories
input_directory = 'data'  # Folder containing <date>.csv files
output_directory = 'outputs'  # Folder where output files will be stored

# Create the output directory if it doesn't exist
os.makedirs(output_directory, exist_ok=True)

# Read the Group.csv file and create a dictionary of identifiers
group_file = 'Group.csv'
group_data = pd.read_csv(group_file)
identifiers = {}

for _, group_row in group_data.iterrows():
    first_name = group_row['first name']
    last_name = group_row['last name']
    full_name = f"{first_name} {last_name}"
    identifiers[full_name] = (first_name, last_name)

# Process each <date>.csv file in the input directory
date_csv_files = glob.glob(os.path.join(input_directory, '*.csv'))
for date_csv_file in date_csv_files:
    output_file = os.path.join(output_directory, f"output_{os.path.splitext(os.path.basename(date_csv_file))[0]}.csv")

    # Process the <date>.csv file and get the rows for the output CSV
    date_rows = process_date_csv(date_csv_file, identifiers)

    # Write the output CSV file to the output directory
    with open(output_file, 'w') as f:
        for row in date_rows:
            f.write(row + "\n")

    print(f'Output CSV file created: {output_file}')