# THOUGHT Outline Processor - Feature Documentation

This document provides a hierarchical description of THOUGHT's features, designed to serve as the foundation for generating test cases.

---

## 1. Overview

THOUGHT is an outline processor and creative thought generator that is part of the SuperROM suite for the TRS-80 Model 100/200. It serves as the central and cohesive unit in a totally integrated word and number tracking system.

### 1.1 Core Purpose

- **Creative thought generator**: Captures numbers, facts, and concepts in every degree of complexity
- **Outline processor**: Organizes information into easily workable outline forms
- **Information organizer**: Manages multiple types of documents in a single accessible format

### 1.2 Main Features

THOUGHT helps organize:

1. **Thinking** - Into an easily workable outline form
2. **Notes** - Into major and minor ideas, or important and varying degrees of less important points
3. **Projects** - Client jobs with correspondence, worksheets, budgets, bid specifications, quotes, and price lists
4. **Information** - Collecting details in one place (e.g., marketing plans with staff assignments, phone numbers, ad copy)
5. **Correspondence** - All letters in one file, organized by client, job, date, or current file status
6. **Details** - Business plans adjusted for different presentations (banker vs. investors)
7. **Speeches, Memos, and Reports** - Including quotes, facts, and ideas from other documents
8. **Lists** - Inventory records, price changes, TO-DO lists in a practical form

### 1.3 Flexibility Features

- **Scalable detail**: Organize information into as much or as little detail as needed at any one time
- **Expandability**: Expand or collapse headings and subheadings
- **Compactability**: Hide subheadings for uncluttered views
- **Clone-ability**: Create linked duplicates that stay synchronized
- **Printability**: Generate various printout formats
- **Integratability**: Works with other SuperROM modules (WRITE ROM, Lucid Spreadsheet, Database)

---

## 2. Core Functionality

### 2.1 Outline Structure

#### 2.1.1 Headings

- **Root headline (title)**: Top-level heading of the outline
- **Major headings**: Primary categories or sections
- **Subheadings**: Nested items under headings
- **Hierarchical levels**: Multiple levels of nesting with visual indicators

#### 2.1.2 Visual Indicators

| Symbol | Meaning |
|--------|---------|
| `+` | Heading with hidden children (can be expanded) |
| `-` | Heading with visible children (can be collapsed) |
| `.` | Document reference (text file) |
| `&` | Cloned heading (twin/mate) |
| (none) | Leaf node / final item |

#### 2.1.3 Document References

- **Text files**: External `.DO` files referenced in the outline
- **Linked documents**: Documents that can be opened and edited inline
- **RAM status**: Documents may be loaded (showing first line) or unloaded (filename only)

---

## 3. Modes of Operation

### 3.1 Create Mode

**Purpose**: Adding new content to the outline

**Entering Create Mode**:
- Press ENTER on a line with `?` cursor indicator

**Create Mode Functions**:
- Create headlines (top-level entries)
- Create headings (major categories)
- Create subheadings (nested items)
- Create document references (link to `.DO` files)

**Exiting Create Mode**:
- Press F8 on an empty line to return to Review/Revise mode

**Create Window**:
- Positioned at cursor location
- Displays `?` to indicate create mode
- Accepts text input for new outline entries

## 4. Navigation Features

### 4.1 Cursor Movement

| Action | Key | Description |
|--------|-----|-------------|
| Move cursor up | ↑ | Navigate to previous line |
| Move cursor down | ↓ | Navigate to next line |
| Move cursor left | ← | Move to parent heading |
| Move cursor right | → | Move to first child |
| Move to first line | CTRL + ↑ | Go to root headline |
| Move to last line | CTRL + ↓ | Go to end of outline |

### 4.2 View Management

| Action | Key | Description |
|--------|-----|-------------|
| Hide children | F2 | Collapse subheadings under current heading |
| Show children | F3 | Expand subheadings under current heading |
| Global show | SHIFT + F3 | Expand all hidden content from cursor to end |
| Global hide | SHIFT + F2 | Hide all content except root headline |

### 4.3 Document Access

| Action | Key | Description |
|--------|-----|-------------|
| Open document | ENTER | Open linked text file for editing |
| Edit document | F6 | Enter document editing mode |
| Exit document | F8 | Return to outline from document |

---

## 5. Content Management Features

### 5.1 Creation Features

#### 5.1.1 Creating Headings and Subheadings

1. Enter Create mode (PRESS ENTER)
2. Type heading text
3. Press ENTER to create
4. Repeat for subheadings (indentation implied by structure)

#### 5.1.2 Creating Document References

1. In Create mode, position cursor where document should be inserted
2. Type filename (e.g., `FILENAME.DO`)
3. Press ENTER
4. The filename appears in the outline
5. To edit, press ENTER on filename to open in document mode

#### 5.1.3 Creating Blank Lines

1. In Create mode, press SPACEBAR then ENTER
2. Creates a blank headline (useful for spacing in printouts)

### 5.2 Deletion Features

| Action | Key Sequence | Description |
|--------|--------------|-------------|
| Delete line | CTRL + DEL BKSP | Delete current line and children |
| Delete document reference | CTRL + DEL BKSP on document line | Removes from outline, keeps document in RAM |

**Behavior**:
- Deleting a heading deletes all its children
- Deleting a document reference from outline keeps the `.DO` file in RAM
- To permanently remove, delete from RAM via DISK+ or Main Menu

### 5.3 Modification Features

#### 5.3.1 Text Editing

- Full text editing on any heading or subheading
- Character-by-character editing
- Line-level operations

#### 5.3.2 Movement (Drag)

| Action | Key | Description |
|--------|-----|-------------|
| Drag heading | F5 | Move heading and all children to new location |

**Drag Features**:
- One-button drag operation
- Moves entire subtree (heading + all children)
- Can move within same outline or between outlines
- Can move to different SuperROM modules

#### 5.3.3 Clone

| Action | Key | Description |
|--------|-----|-------------|
| Clone entry | F4 | Create exact duplicate that stays linked |

**Clone Behavior**:
- Creates duplicate(s) with `&` indicator
- Changes to one clone affect all clones
- Use for items that appear in multiple locations
- Examples: catalog items under multiple categories, staff under different projects

**Clone vs Copy**:
- **Clone**: Linked duplicates; change one, all change
- **Copy**: Independent duplicate; changes don't affect original

#### 5.3.4 Sort

| Action | Key | Description |
|--------|-----|-------------|
| Sort alphabetically | F6 | Sort subheadings alphabetically |

**Sorting**:
- Sorts immediate children of current heading
- Alphabetical order (A-Z)
- Preserves hierarchy

### 5.4 Copy and Paste

**Integration**: THOUGHT shares copy/paste with:
- WRITE ROM
- Lucid Spreadsheet
- Database
- Other outlines

**Copy/Paste Operations**:
1. Select range (F7 - Sel)
2. Copy (F5 - Copy) or Cut (F6 - Cut)
3. Navigate to destination
4. Paste (F4 - Paste)

---

## 6. File Management Features

### 6.1 Outline Files

- **Extension**: `.CT` (Creative Thought)
- **Storage**: Disk or cassette
- **Persistence**: Saves entire outline structure and content

### 6.2 Document Files

- **Extension**: `.DO` (Document)
- **Storage**: Disk or cassette
- **Size**: Limited by available RAM

### 6.3 File Operations

| Action | Description |
|--------|-------------|
| Save outline | Save to disk/cassette as `.CT` file |
| Load outline | Load from disk/cassette |
| Save document | Save from RAM to disk/cassette as `.DO` file |
| Load document | Load from disk/cassette into RAM |

**RAM Management**:
- Only open documents consume RAM
- Multiple documents can be referenced but only some loaded
- Unloaded documents show as filename only in outline

---

## 7. Print Features

### 7.1 Print Destination Options

| Destination | Description |
|-------------|-------------|
| Parallel printer | Default printer connection |
| Serial printer | Alternative printer connection |
| RAM file | Output to memory buffer |
| Another computer | Network/terminal output |

### 7.2 Print Format Options

| Setting | Default | Description |
|---------|---------|-------------|
| Spacing | Single | Line spacing |
| Pause between pages | No pause | Page separation |
| CRLF (Carriage Return Line Feed) | OFF | Line termination |
| Left margin | 8 spaces | Left indentation |
| Right margin | 74 spaces | Right boundary |
| Page numbering | ON | Page footers |
| Indentation | 4 spaces per level | Hierarchical indentation |
| Automatic numbering | OFF | Heading prefixes |
| Expansion | Completely expanded | All content included |
| Extra line spacing | No extra lines | Vertical spacing |

### 7.3 Print Commands

| Action | Key | Description |
|--------|-----|-------------|
| Print | PRINT | Initiate print sequence |
| Go | F1 | Execute print with current settings |
| Output | F2 | Change output destination |
| Line spacing | F3 | Toggle line spacing |
| Pause | F4 | Toggle page pause |
| CRLF | F5 | Toggle carriage return |
| Set | F6 | Set new print defaults |
| Exit | F8 | Cancel print and return |

### 7.4 Print Default Management

- **Save defaults**: Changes become new defaults for current outline
- **Persistent**: Defaults saved with outline file
- **Per-outline**: Each outline can have unique print settings
- **Quick print**: Two-button print (PRINT + F1) for default settings

---

## 8. Integration Features

### 8.1 SuperROM Integration

THOUGHT is one of four integrated programs:

1. **THOUGHT**: Outline processor
2. **WRITE ROM**: Word processing
3. **Lucid Spreadsheet**: Spreadsheet calculations
4. **Database**: Data management

### 8.2 Cross-Module Copy/Paste

| Source | Destination | Supported |
|--------|-------------|-----------|
| THOUGHT | WRITE ROM | Yes |
| THOUGHT | Lucid | Yes |
| THOUGHT | Database | Yes |
| THOUGHT | THOUGHT (other) | Yes |
| WRITE ROM | THOUGHT | Yes |
| Lucid | THOUGHT | Yes |
| Database | THOUGHT | Yes |

---

## 9. Advanced Features

### 9.1 Multiple View Management

| Action | Key | Description |
|--------|-----|-------------|
| Hide level | F2 | Collapse to single line |
| Show level | F3 | Expand children |
| Global show | SHIFT + F3 | Expand all from cursor |
| Global hide | SHIFT + F2 | Hide all but root |

### 9.2 Outline Management

**Outline Types**:
1. **Business plans**: Objectives, strategies, actions
2. **Project plans**: Tasks, resources, timelines
3. **Document management**: Master file catalog
4. **Correspondence**: Letter tracking and organization
5. **Notes**: Hierarchical note-taking
6. **Reports**: Structured information presentation

### 9.3 Application Scenarios

#### 9.3.1 Business Plan

- **Structure**: Objectives → Strategies → Actions
- **Features used**: Hierarchical outlining, cloning for repeated items
- **Output**: Print with indentation for presentation

#### 9.3.2 Document File Management

- **Master outline**: Catalog all documents
- **Categories**: Personal, Business, Employees, Suppliers, Customers
- **Document references**: Filename-only entries for unopened files
- **Disk integration**: Load/unload documents as needed

#### 9.3.3 Writing Projects

- **Structure**: Book/essay outline with chapter files
- **Document files**: Individual `.DO` files for each section
- **Print**: Export to Write ROM for final formatting

---

## 10. Function Key Summary

### 10.1 Create Mode

| Key | Function |
|-----|----------|
| ENTER | Create new entry / exit to Review/Revise |
| F8 | Exit to Review/Revise mode |

### 10.2 Review/Revise Mode

| Key | Function |
|-----|----------|
| ↑↓←→ | Cursor movement |
| F2 | Hide children |
| F3 | Show children |
| F4 | Clone |
| F5 | Drag (move) |
| F6 | Sort alphabetically |
| F7 | Select range |
| F8 | Exit to previous mode |
| SHIFT + F2 | Global hide |
| SHIFT + F3 | Global show |

### 10.3 Document Mode

| Key | Function |
|-----|----------|
| F6 | Enter document |
| F8 | Exit to outline |

### 10.4 Print Mode

| Key | Function |
|-----|----------|
| F1 | Go (print) |
| F2 | Output destination |
| F3 | Line spacing |
| F4 | Page pause |
| F5 | CRLF |
| F6 | Set new defaults |
| F8 | Exit |

---

## 11. Test Case Generation Guidelines

### 11.1 Test Categories

1. **Mode Transition Tests**: Verify transitions between Create and Review/Revise modes
2. **Navigation Tests**: Cursor movement and view management
3. **Content Creation Tests**: Creating headings, subheadings, documents
4. **Content Modification Tests**: Editing, deleting, moving, cloning
5. **File Operation Tests**: Save/load outlines and documents
6. **Print Tests**: Various print configurations and destinations
7. **Integration Tests**: Copy/paste between modules
8. **Edge Case Tests**: Empty outlines, deeply nested structures, large documents

### 11.2 Test Scenario Patterns

- **Happy path**: Complete workflow without errors
- **Error handling**: Invalid inputs, missing files, insufficient RAM
- **State preservation**: Settings retained after save/load
- **Clone behavior**: Changes propagate to all clones
- **RAM management**: Document load/unload behavior
- **Hierarchical integrity**: Parent-child relationships maintained

---

## 12. Technical Specifications

### 12.1 File Formats

| File Type | Extension | Content |
|-----------|-----------|---------|
| Outline | `.CT` | Full outline structure with all content |
| Document | `.DO` | Plain text document |
| Text | `.TX` | Text file (alternative extension) |

### 12.2 RAM Constraints

- Document files limited by available RAM
- Multiple documents can be referenced but only some loaded at once
- Outline content always in RAM when open

### 12.3 Platform Support

- **Primary**: TRS-80 Model 100
- **Secondary**: TRS-80 Model 200
- **Emulation**: Virtual T (PC-8201a via socket/telnet)