# miRcore_attendance

## Usage
Download `run` (not `run.sh` or any others), and execute the file in your terminal with `<path to dir>/run`

##### Settings
* Group.csv file: A csv file containing the roll call exported from Google Sheets. The first and last names of each person must be in the fourth and fifth columns respectively, as shown below. Each person is assigned identifiers<sup>1</sup> determined by the uniqueness of their names.
* Date.csv files: File(s) exported from Zoom attendance logs. For each file (or Zoom meeting), each name found on Zoom is looked up on the roll call (Group.csv) to determine whether that person was fully present<sup>2</sup>.
* Output directory: Where to write the output
* Output filename: Name of the output file
* Time threshold: Minimum time for an attendee to be present in Zoom to be considered a full match. 
* Include GLs: Include zoom names that have "UM", "GIDAS", or "GL" in their name. False by default (i.e. skips GLs when unchecked)
* Developer Mode: Outputs Zoom name for all full and partial matches (rather than just true for full matches), and always specify the type of match.

## Implementation details
<sup>1</sup> Identifiers include a person's first name, last name, and full name if both the first and last names are unique. Otherwise, a person will only be distinguished with certainty by name by whichever is unique.
<sup>2</sup> Each Zoom name has three possible classifications:
* Full matches: a hit with a (unique) identifier and a time present that is greater than the time threshold. Considered fully present, and the output will be "TRUE". This takes precedence over a partial match.
* Partial matches: a hit with a first and/or last name, but not an identifier (and thus not unique to that person in the roll call). In other words, the person on Zoom may or may not be this person in the roll call, so the output is "Partial: \<Zoom name\> <(time present)>" and will require a decision from the user.
* Misses: if the Zoom name doesn't match with any person on the roll call (fully or partially), the Zoom name will be listed at the end of the output.

Note: each person in the roll call (Group.csv) is designated a "FALSE" value by default. Thus, if a person doesn't get either a full match or any partial matches after parsing the Zoom log, it will remain FALSE and they will be considered absent.

## Example
1. A Google Sheet with a roll call contains the following students (first, last):
* Anan, Nuengchana
* Vishal, John
* John, Li
* John, Smith
* Inhan, Lee
2. A Zoom attendance log (Feb11.csv) contains the following attendees (and how long they where present in the meeting)
* John (110)
* Vishal (120)
* Anan Nuengchana (5)
* John Wagner (115)
* Kat Mudge (60)
* GL Mayee (60)
3. With the ouput settings on default, a time threshold of 10 min, exclusion of GLs, and non-developer mode, the program would write an "output.tsv" file in the current directory with the following content:
Feb11\t
Partial: Anan Nuengchana (5)\t
TRUE\t
Partial: John (110), John Wagner (115)\t
Partial: John (110), John Wagner (115)\t
FALSE\t
\t
Misses:\t
Kat Mudge (60)\t


The first name found on Zoom, John, partially matches Vishal John, John Li, and John Smith because John is not a unique first or last name.
The second, Vishal, matches fully with Vishal John because Vishal is unique among all first and last names. Since this takes precedence over partial matches, the row corresponding to Vishal is overwritten as TRUE.
The third, Anan Nuengchana, matches with a unique identifier, but is below the time threshold and is thus marked as a partial match.
The fourth, John Wagner, also partially matches with John Li and John Smith.
The fifth, Kat Mudge, is not fully or partially matched, so it is listed at the end of the output as a miss.
The sixth, GL Mayee, contains "GL" and is thus not looked up in the roll call.

Since Inhan Lee was never matched (fully or partially) by any name on the Zoom meeting, the output value is left as default (FALSE).

The output can then be imported into Google Sheets with conditional formatting to automatically fill out checkboxes.




## Future Plans


## Contact
If you have any questions, concerns, or bug reports, please don't hesitate to contact Anan (ananueng@umich.edu). Feel free to also send any criticism or feedback!