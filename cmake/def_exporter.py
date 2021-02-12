import json
import logging
import os
import re

try:
    import appdirs
except ImportError:
    print("Could not load appdirs, please install with 'python -m pip install appdirs'")
    exit(0)


def extract_component_codes(base_path):
    codes_found = {}

    for line in open(os.path.join(base_path, 'inc', 'opendefs.h'), 'r'):
        m = re.search(' *COMPONENT_([^ .]*) *= *(.*), *', line)
        if m:
            name = m.group(1)
            try:
                code = int(m.group(2), 16)
            except ValueError:
                logging.error("component '{}' - {} is not a hex number".format(name, m.group(2)))
            else:
                logging.debug("extracted component '{}' with code {}".format(name, code))
                codes_found[code] = name

    return codes_found


def extract_log_descriptions(base_path):
    codes_found = {}
    for line in open(os.path.join(base_path, 'inc', 'opendefs.h'), 'r'):
        m = re.search(' *ERR_.* *= *([xXA-Fa-f0-9]*), *// *(.*)', line)
        if m:
            desc = m.group(2).strip()
            try:
                code = int(m.group(1), 16)
            except ValueError:
                logging.error("log description '{}' - {} is not a hex number".format(desc, m.group(2)))
            else:
                logging.debug("extracted log description '{}' with code {}".format(desc, code))
                codes_found[code] = desc

    return codes_found


def extract_6top_rcs(base_path):
    # find sixtop return codes in sixtop.h

    codes_found = {}
    for line in open(os.path.join(base_path, 'openstack', '02b-MAChigh', 'sixtop.h'), 'r'):
        m = re.search(' *#define *IANA_6TOP_RC_([^ .]*) *([xXA-Za-z0-9]+) *// *([^ .]*).*', line)
        if m:
            name = m.group(3)
            try:
                code = int(m.group(2), 16)
            except ValueError:
                logging.error("return code '{}': {} is not a hex number".format(name, m.group(2)))
            else:
                logging.debug("extracted 6top RC '{}' with code {}".format(name, code))
                codes_found[code] = name

    return codes_found


def extract_6top_states(base_path):
    # find sixtop state codes in sixtop.h

    codes_found = {}
    for line in open(os.path.join(base_path, 'openstack', '02b-MAChigh', 'sixtop.h'), 'r'):
        m = re.search(' *SIX_STATE_([^ .]*) *= *([^ .]*), *', line)
        if m:
            name = m.group(1)
            try:
                code = int(m.group(2), 16)
            except ValueError:
                logging.error("state '{}' - {} is not a hex number".format(name, m.group(2)))
            else:
                logging.debug("extracted 6top state '{}' with code {}".format(name, code))
                codes_found[code] = name

    return codes_found


if __name__ == "__main__":
    base_path = os.path.abspath('..')

    app_dir = appdirs.user_data_dir("openvisualizer")

    with open(os.path.join(app_dir, 'component-codes.json'), 'w') as f:
        json.dump(extract_component_codes(base_path), f)

    with open(os.path.join(app_dir, 'log-descriptions.json'), 'w') as f:
        json.dump(extract_log_descriptions(base_path), f)

    print("Parsed opendefs.h")

    with open(os.path.join(app_dir, 'sixtop-rcs.json'), 'w') as f:
        json.dump(extract_6top_rcs(base_path), f)

    with open(os.path.join(app_dir, 'sixtop-states.json'), 'w') as f:
        json.dump(extract_6top_states(base_path), f)

    print("Parsed sixtop.h")
