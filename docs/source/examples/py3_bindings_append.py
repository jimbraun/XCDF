from pathlib import Path
from pprint import pprint
import numpy as np
from xcdf import File

test_file = Path.cwd() / Path("example_file_py3bindings.xcd")

# You can define field properties as tuples
# to re-use them when appending multiple times
field_A_properties = ("A", 1, "")
field_B_properties = ("B", 0.1, "A")


# Re-open the file, but in 'append' mode
with File(str(test_file), "a") as same_file:
    # Re-allocate the same data field but for this file

    field_A = same_file.allocate_uint_field(*field_A_properties)
    field_B = same_file.allocate_float_field(*field_B_properties)

    # Add data for the second event/row

    field_A.add(1)

    field_B.add(5)

    same_file.write()

# Let's add more data
with File(str(test_file), "a") as again_same_file:
    # Re-allocate the same data field but for this file

    field_A = again_same_file.allocate_uint_field(*field_A_properties)
    field_B = again_same_file.allocate_float_field(*field_B_properties)

    # Add data for the second event/row

    field_A.add(10)
    field_B.add(np.linspace(-5.0, 5.0, 10))

    again_same_file.write()

# Read back and print contents
events = {}
with File(str(test_file), "r") as input_file:
    file_header = input_file.comments
    for event_index, event in enumerate(input_file):
        events[event_index] = event

print("FILE HEADER\n===========\n")
print(file_header)

print("\nEVENTS\n======\n")
pprint(events, width=5)
