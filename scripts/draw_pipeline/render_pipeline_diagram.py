#!/usr/bin/env python3

import json
import sys
from graphviz import Digraph

MAX_PARAM_LEN = 60

def truncate_value(val):
    s = str(val)
    if len(s) > MAX_PARAM_LEN:
        return s[:MAX_PARAM_LEN-3] + "..."
    return s

def format_label(stage):
    params = stage.get("parameters", {})
    if params:
        param_rows = "".join(
            f"<TR><TD ALIGN='LEFT'>{k}</TD><TD ALIGN='LEFT'>{truncate_value(v)}</TD></TR>"
            for k, v in params.items()
        )
    else:
        param_rows = "<TR><TD ALIGN='LEFT' COLSPAN='2'>None</TD></TR>"

    label = f"""<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR><TD COLSPAN="2" BALIGN="CENTER"><B>{stage['id']}</B><BR/>{stage['type']}</TD></TR>
      <TR><TD COLSPAN="2" BALIGN="LEFT"><I>Parameters</I></TD></TR>
      {param_rows}
    </TABLE>
    >"""
    return label

def render_pipeline(json_path, output_path="pipeline_diagram", format="png"):
    with open(json_path, "r") as f:
        config = json.load(f)

    dot = Digraph(comment="Pipeline Diagram", format=format)
    dot.attr("node", shape="none", fontname="Helvetica")

    id_to_stage = {stage["id"]: stage for stage in config["pipeline"]}
    all_next_ids = set()
    for stage in config["pipeline"]:
        all_next_ids.update(stage.get("next", []))

    all_ids = set(id_to_stage.keys())
    entry_points = all_ids - all_next_ids

    for stage in config["pipeline"]:
        is_entry = stage["id"] in entry_points
        is_exit = not stage.get("next")

        if is_entry:
            fillcolor = "lightblue"
        elif is_exit:
            fillcolor = "lightgreen"
        else:
            fillcolor = "lightgray"

        dot.node(stage["id"], label=format_label(stage), style="filled", fillcolor=fillcolor)

    for stage in config["pipeline"]:
        for next_id in stage.get("next", []):
            dot.edge(stage["id"], next_id)

    dot.render(output_path, view=False)
    print(f"Pipeline diagram saved to: {output_path}.{format}")

if __name__ == "__main__":
    default_json_path = "/home/jack/cpp_projects/analysis_pipeline_project/apps/nearline/config/modes/default.json"

    json_path = sys.argv[1] if len(sys.argv) > 1 else default_json_path
    output_path = sys.argv[2] if len(sys.argv) > 2 else "pipeline_diagram"
    format = sys.argv[3] if len(sys.argv) > 3 else "png"

    print(f"Using config: {json_path}")
    render_pipeline(json_path, output_path, format)
