# Rsd

RenderSpud Data, developed originally as a scene description format for production rendering.  It is human-readable and highly composable.

Rsd uses modern C++ and the lemon parser generator.

## History

The file format was inspired by POD, as used in The Bakery's Relight renderer.  At Tippett Studio, Mike Farnsworth created a clean-room implementation of the POD format, which was used to both interact with Relight as well as utilized independently in the production asset pipeline quite successfully.  It contained a C++ and python API that would parse and allow quick easy access to structures within the file along with resolving references and includes.

There were some ambiguities and limitations in the format, however, and so a successor format and API was created by Mike Farnsworth.  It never saw production use at Tippett Studio, as it was too hard to move away from the original POD-based pipeline software and data.  When Mike Farnsworth moved over to Solid Angle to work on the Arnold renderer, Tippett Studio gave him its blessing in taking the new format and library with him, and all associated rights.  Rsd is the cleaned-up and improved version of that project.

## Features

### File format

* Straightforward, human-readable format
* Value types:
 * Block (with named values)
 * Array (list of unnamed values), nestable
 * Simple types: int, float
 * Strings (with ${...} to embed references)
 * Macros (with keyword-value arguments)
 * References to other values
* Highly referential
 * Strings can reference other values, which are turned into textual content if possible
 * Namespaces/paths and subscripts: someBlock.someInnerBlock.arrayValue[anotherBlock["someIntValue"]]
 * Includes in any block (or in the main scope)
 * Blocks may "inherit" from another block and have all the parent block's values, and override same-named values
* Types may be optionally assigned to values

### API

* Parse full Rsd data or just references from files, strings, streams
* Write Rsd data to files, strings, and streams
* Create and populate new values and Rsd data programmatically
* Convert values from one type to another
* Resolve references to get final values where possible
* Create macros for dynamically creating values, referenced from Rsd data
* (WIP) Create and apply schemas for validating Rsd data against known types
