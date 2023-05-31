from pathlib import Path
import numpy as np
from xcdf import File

# Open a new file for writing
test_file = Path.cwd() / Path("example_file_py3bindings.xcd")
with File(str(test_file), "w") as f:
    # Define the data fields

    field_A = f.allocate_uint_field("A", 1, "")
    field_B = f.allocate_float_field("B", 0.1, "A")

    # Add data for the first event/row

    field_A.add(4)

    # Field "B" is a child of "A", so it needs to contain
    # as many values as the entry in "A" says for this event
    # Let's add the numpy equivalent of [0.  , 0.25, 0.5 , 0.75]
    field_B.add(np.arange(0, 1, 0.25))

    f.write()

    # Add data for the second event/row

    field_A.add(1)
    field_B.add(6.0)

    f.write()

    # Add a final comment in the file header
    f.add_comment("This is a test comment")
