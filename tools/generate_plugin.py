#!/usr/bin/env python3
"""
TinyCLI Plugin Generator

This script generates skeleton code for TinyCLI plugins.
"""

import os
import sys
import json
import argparse
from pathlib import Path

# Template for plugin JSON configuration
JSON_TEMPLATE = """{{
  "name": "{name}",
  "description": "{description}",
  "version": "{version}",
  "commands": [
{commands}
  ],
  "library": "lib{name}_plugin.so"
}}
"""

# Template for a command in JSON
COMMAND_JSON_TEMPLATE = """    {{
      "name": "{name}",
      "view": "{view}",
      "description": "{description}",
      "handler": "{handler}"
    }}"""

# Template for plugin C implementation
C_TEMPLATE = """/**
 * @file {name}_plugin.c
 * @brief {description}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "tinycli_plugin.h"
#include "tinycli_view.h"

/* Forward declarations */
{forward_declarations}

/* Plugin commands */
static tinycli_cmd_t commands[] = {{
{commands}
}};

/**
 * Plugin initialization
 */
static int {name}_init(void *user_data)
{{
    printf("Initializing {name} plugin\\n");
    return 0;
}}

/**
 * Plugin cleanup
 */
static int {name}_cleanup(void *user_data)
{{
    printf("Cleaning up {name} plugin\\n");
    return 0;
}}

/**
 * Get plugin commands
 */
static int {name}_get_commands(tinycli_cmd_t **cmds, int *num_cmds)
{{
    if (!cmds || !num_cmds) {{
        return -1;
    }}
    
    *cmds = commands;
    *num_cmds = sizeof(commands) / sizeof(commands[0]);
    
    return 0;
}}

/**
 * Define plugin
 */
TINYCLI_PLUGIN_DEFINE({name}_init, {name}_cleanup, {name}_get_commands);

{implementations}
"""

# Template for command handler forward declaration
HANDLER_DECLARATION_TEMPLATE = "static int {handler}(tinycli_cmd_ctx_t *ctx);"

# Template for command in C structure
COMMAND_C_TEMPLATE = """    {{
        .name = "{name}",
        .description = "{description}",
        .view = {view},
        .handler = {handler},
        .params = NULL,
        .num_params = 0,
        .changes_view = {changes_view},
        .target_view = {target_view}
    }}"""

# Template for command handler implementation
HANDLER_IMPLEMENTATION_TEMPLATE = """/**
 * Handle {name} command
 */
static int {handler}(tinycli_cmd_ctx_t *ctx)
{{
    /* Unused parameter */
    (void)ctx;
    
    printf("{name} command executed\\n");
    return 0;
}}
"""

def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(description='Generate TinyCLI plugin skeleton')
    parser.add_argument('name', help='Plugin name')
    parser.add_argument('--description', '-d', default='Plugin for TinyCLI',
                        help='Plugin description')
    parser.add_argument('--version', '-v', default='1.0.0',
                        help='Plugin version')
    parser.add_argument('--output-dir', '-o', default='.',
                        help='Output directory')
    parser.add_argument('--commands', '-c', nargs='+', default=[],
                        help='Commands to include in the plugin (format: name:view:description)')
    return parser.parse_args()

def generate_plugin(args):
    """Generate plugin skeleton code."""
    name = args.name.lower()
    description = args.description
    version = args.version
    output_dir = Path(args.output_dir)
    
    # Create output directory if it doesn't exist
    plugin_dir = output_dir / name
    plugin_dir.mkdir(parents=True, exist_ok=True)
    
    # Parse commands
    commands = []
    for cmd_str in args.commands:
        parts = cmd_str.split(':')
        if len(parts) >= 3:
            cmd_name = parts[0]
            cmd_view = parts[1]
            cmd_desc = parts[2]
            cmd_handler = f"{name}_{cmd_name}_handler"
            
            commands.append({
                'name': cmd_name,
                'view': cmd_view,
                'description': cmd_desc,
                'handler': cmd_handler,
                'changes_view': 'false',
                'target_view': cmd_view
            })
        else:
            print(f"Warning: Invalid command format: {cmd_str}")
    
    # If no commands specified, add a default one
    if not commands:
        commands.append({
            'name': f"{name}",
            'view': 'VIEW_PRIVILEGED',
            'description': f"Execute {name} command",
            'handler': f"{name}_handler",
            'changes_view': 'false',
            'target_view': 'VIEW_PRIVILEGED'
        })
    
    # Generate JSON configuration
    json_commands = []
    for i, cmd in enumerate(commands):
        json_cmd = COMMAND_JSON_TEMPLATE.format(
            name=cmd['name'],
            view=cmd['view'].lower() if cmd['view'].startswith('VIEW_') else cmd['view'],
            description=cmd['description'],
            handler=cmd['handler']
        )
        if i < len(commands) - 1:
            json_cmd += ','
        json_commands.append(json_cmd)
    
    json_content = JSON_TEMPLATE.format(
        name=name,
        description=description,
        version=version,
        commands='\n'.join(json_commands)
    )
    
    # Generate C implementation
    forward_declarations = []
    c_commands = []
    implementations = []
    
    for i, cmd in enumerate(commands):
        # Forward declaration
        forward_declarations.append(
            HANDLER_DECLARATION_TEMPLATE.format(handler=cmd['handler'])
        )
        
        # Command structure
        c_cmd = COMMAND_C_TEMPLATE.format(
            name=cmd['name'],
            description=cmd['description'],
            view=cmd['view'],
            handler=cmd['handler'],
            changes_view=cmd['changes_view'],
            target_view=cmd['target_view']
        )
        if i < len(commands) - 1:
            c_cmd += ','
        c_commands.append(c_cmd)
        
        # Handler implementation
        implementations.append(
            HANDLER_IMPLEMENTATION_TEMPLATE.format(
                name=cmd['name'],
                handler=cmd['handler']
            )
        )
    
    c_content = C_TEMPLATE.format(
        name=name,
        description=description,
        forward_declarations='\n'.join(forward_declarations),
        commands='\n'.join(c_commands),
        implementations='\n'.join(implementations)
    )
    
    # Write files
    with open(plugin_dir / f"{name}.json", 'w') as f:
        f.write(json_content)
    
    with open(plugin_dir / f"{name}_plugin.c", 'w') as f:
        f.write(c_content)
    
    # Generate CMakeLists.txt
    cmake_content = f"""# CMakeLists.txt for {name} plugin

# Add include directories
include_directories(${{CMAKE_CURRENT_SOURCE_DIR}}/../../include)

# Add plugin library
add_library({name}_plugin MODULE {name}_plugin.c)

# Set output properties
set_target_properties({name}_plugin PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${{CMAKE_BINARY_DIR}}/plugins"
)

# Install plugin
install(TARGETS {name}_plugin
    LIBRARY DESTINATION lib/tinycli/plugins
)

# Install plugin configuration
install(FILES {name}.json
    DESTINATION share/tinycli/plugins
)
"""
    
    with open(plugin_dir / "CMakeLists.txt", 'w') as f:
        f.write(cmake_content)
    
    print(f"Generated plugin '{name}' in {plugin_dir}")
    print(f"Files created:")
    print(f"  - {plugin_dir}/{name}.json")
    print(f"  - {plugin_dir}/{name}_plugin.c")
    print(f"  - {plugin_dir}/CMakeLists.txt")
    print("\nTo build and install the plugin:")
    print(f"  1. Add 'add_subdirectory({name})' to plugins/CMakeLists.txt")
    print(f"  2. Run 'mkdir -p build && cd build && cmake .. && make'")
    print(f"  3. The plugin will be built as 'build/plugins/lib{name}_plugin.so'")

if __name__ == "__main__":
    args = parse_args()
    generate_plugin(args)
