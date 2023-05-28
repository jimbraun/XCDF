from pathlib import Path
from pprint import pprint
from xcdf import File

test_file = Path.cwd() / Path("example_file_py3bindings.xcd")

events = {}

with File(str(test_file), "r") as input_file:
    file_header = input_file.comments
    for event_index, event in enumerate(input_file):
        events[event_index] = event

print("FILE HEADER\n===========\n")
print(file_header)

print("\nEVENTS\n======\n")
pprint(events, width=5)
