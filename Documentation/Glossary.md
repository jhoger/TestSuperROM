# THOUGHT Glossary

This document defines key terms used in the THOUGHT outline processor.

---

## A

### ASCII
American Standard Code for Information Interchange. A character encoding standard used for text files. THOUGHT uses ASCII format for document files.

### Automatic Numbering
A print formatting option where heading levels are automatically prefixed with numbers (e.g., 1., 1.1., 1.1.1.). Can be enabled/disabled in print settings.

---

## B

### BREAK/PAUSE Key
A command key on the Model 100/200 keyboard. Used to temporarily stop printing (PAUSE) or abort operations (SHIFT + BREAK).

### Blank Line
A headline entry that contains no visible text. Created in Create mode by pressing SPACEBAR then ENTER. Used for visual spacing in printouts.

---

## C

### Carriage Return Line Feed (CRLF)
A control sequence for text file line termination. Model 100 normally sends only Carriage Return (CR). The CRLF setting (F6 in Print mode) adds Line Feed (LF) for compatibility with non-Radio Shack printers.

### Clone
A feature where duplicate entries are created that remain linked. Changes to one clone propagate to all others. Indicated by `&` symbol in the outline. Contrast with Copy.

### Copy
Creating an independent duplicate of text or content. Unlike Clone, changes to a Copy do not affect the original. Part of the Cut/Copy/Paste system.

### CTRL + DEL BKSP
A key combination to delete a line and all its children in the outline. Also used to delete document references from the outline while keeping the document in RAM.

### CTRL + ↑ (Up Arrow)
Cursor movement command that jumps to the root headline (top of outline).

### CTRL + ↓ (Down Arrow)
Cursor movement command that jumps to the end of the outline.

---

## D

### Data
A mode in THOUGHT that allows access to external files (like `.DO` documents).

### Del BKSP
A key on Model 100/200 that combines delete and backspace functionality. Used with CTRL for deletion operations.

### Document (.DO file)
A text file referenced by or embedded in a THOUGHT outline. Document files can be created and edited within the outline or loaded from disk.

### Document Mode
A mode in THOUGHT where you can edit the text content of a document referenced in the outline. Entered by pressing ENTER on a document reference or F6 from outline mode.

---

## E

### Expansion
A print setting that controls how the outline is displayed. Options include "Completely expanded" (all content shown) or collapsed views.

---

## F

### F1 (Go)
In Print mode, initiates the printing process with current settings.

### F2 (Rt/Output/Paus)
Multiple functions depending on context:
- Print mode: Sets right margin
- Print mode: Changes output destination
- Print mode: Toggles page pause between pages

### F3 (Asc/Line Spacing/Show)
Multiple functions:
- Print mode: Toggles ascending/descending sort order
- Print mode: Toggles line spacing (single/double)
- Outline mode: Shows/hides children under current heading

### F4 (Desc/Clone)
Multiple functions:
- Print mode: Sets descending sort order
- Outline mode: Clones the current entry

### F5 (Key/Sort/Drag)
Multiple functions:
- Print mode: Sets key cell for sorting
- Sort feature: Designates the sort key
- Outline mode: Drags/moves the current entry

### F6 (CRLF/Data/Sort)
Multiple functions:
- Print mode: Toggles CRLF (Carriage Return Line Feed)
- Outline mode: Sorts subheadings alphabetically
- Document mode: Enters document editing

### F7 (Set/Sel)
Multiple functions:
- Print mode: Opens advanced print options (Left, Rt, Ftr, Indt, Nbrs, Last, Xtra)
- Outline mode: Selects range for copy/cut operations

### F8 (Exit)
A universal exit function that returns to the previous mode or menu level.

---

## G

### Global Hide
A view management command (SHIFT + F2) that hides all content in the outline except the root headline.

### Global Show
A view management command (SHIFT + F3) that expands all hidden content from the cursor position to the end of the outline.

---

## H

### Headings
Entries in an outline that can contain subheadings. Headings are hierarchical and can be expanded/collapsed. Indicated by `+` (hidden children) or `-` (visible children).

### Hierarchy
The parent-child relationship structure in outlines where headings contain subheadings. Maintains logical organization of content.

---

## I

### Indentation
The visual spacing that indicates nesting levels in an outline. Default is 4 spaces per level. Can be adjusted in print settings.

---

## K

### Key Cell
In sorting operations, the cell used as the basis for sorting. For row sorting, it's any cell in a column. For column sorting, it's any cell in a row.

---

## L

### Left Margin
The horizontal distance from the left edge of the page to the start of printed content. Default is 8 spaces. Configurable in print settings (F1 - Left).

### Line Spacing
Print setting that controls vertical spacing between lines. Options: single spacing or double spacing. Toggled with F3 (Line Spacing) in Print mode.

---

## M

### Master Outline
A THOUGHT outline used as a catalog or directory for managing multiple document files. Can contain references to `.DO` files without loading them into RAM.

### Model T
Generic term for TRS-80 Model 100/200 series laptops. The platform for THOUGHT.

### Multi-file Copy/Cut/Paste
The ability to copy or cut content from one file and paste it into another, including between different THOUGHT outlines.

---

## N

### Navigation
Cursor movement operations in THOUGHT using arrow keys, SHIFT keys, and function keys to move through the outline structure.

---

## O

### Outline
The primary data structure in THOUGHT. A hierarchical arrangement of headings and subheadings that can include document references.

### Outline File (.CT file)
A THOUGHT file containing the complete outline structure and content.

---

## P

### Page Numbering
Print setting that adds page numbers to output. Enabled by default.

### Page Pause
Print setting that stops printing between pages to allow manual paper feeding. Toggled with F5 (Paus) in Print mode.

### Paste
Inserting copied or cut content at the current cursor position. Supports pasting from THOUGHT outlines.

### PC-8201a
A variant of the TRS-80 Model 100 sold in Japan. Supported by Virtual T emulator for testing THOUGHT.

### Print
The process of outputting outline or document content to a printer, file, or other destination. Accessible via PRINT key or function key menu.

### Print Defaults
Settings that become the default for future prints of the same outline. Changes persist in the `.CT` file.

### Print Format Options
Configurable settings for print output:
- Spacing (single/double)
- Page pause
- CRLF
- Left/Right margins
- Page numbering
- Indentation
- Automatic numbering
- Expansion level
- Extra line spacing

### Print Mode
A mode in THOUGHT for configuring and executing print operations. Accessed via PRINT key.

### Project Plan
A type of outline structure for managing tasks, resources, and timelines. One of the primary application categories for THOUGHT.

---

## Q

### Quick Print
A two-button print operation (PRINT + F1) that uses current defaults to quickly initiate printing.

---

## R

### RAM
Random Access Memory. The working memory of the Model 100/200. Document files consume RAM when loaded; outlines are always in RAM when open.

### Range
A selected group of lines in THOUGHT. Specified by line range.

### Review/Revise Mode
The default mode in THOUGHT for viewing, navigating, editing, and organizing the outline. Contrast with Create mode.

---

## S

### SERIAL PRINTER
A printer connected via the serial port of the Model 100/200. Alternative to parallel printer for THOUGHT output.

### SHIFT + BREAK
A key combination to abort printing or other operations and return to the main function key level.

### SHIFT + F2 (Global Hide)
Hides all content in the outline except the root headline.

### SHIFT + F3 (Global Show)
Expands all hidden content from the cursor position to the end of the outline.

### Single Sheet Feeding
A print mode where the printer pauses between pages to allow manual paper feeding. Controlled by the Page Pause feature.

### Sort
A feature in THOUGHT that arranges subheadings alphabetically.

### Subheadings
Nested entries under headings. Can contain further subheadings, creating a multi-level hierarchy.

---

## T

### TAB Key
A navigation and input key. In THOUGHT, used for creating outline hierarchy and as a shortcut for cursor movement.

### TABS
Indentation markers in text that create hierarchy when pasted into THOUGHT. Each TAB becomes one level of indentation.

### TEXT
Text editor application that creates and edits plain text files. Can be used with THOUGHT via Copy/Paste operations.

### THOUGHT
The outline processor. Also the name of the software program and its file extension (.CT).

### THOUGHT (.CT file)
A file containing an outline with all its headings, subheadings, and document references.

### Think Tank
A different outline processor program. THOUGHT can import Think Tank files via PASTE operation.

### TRS-80
Tandy Radio Shack Electronics 80. The computer series for which THOUGHT was designed.

---

## U

### Update
To refresh or reload content. THOUGHT can reload document files from disk without recreating the outline.

---

## V

### View Management
Commands to show or hide outline levels. Includes Hide children (F2), Show children (F3), Global show (SHIFT + F3), and Global hide (SHIFT + F2).

---

## X

### Xtra Line Spacing
A print setting (F7 - Xtra) that adds extra blank lines between headings for better readability.

---

## Visual Indicators

| Symbol | Meaning in THOUGHT |
|--------|-------------------|
| `+` | Heading with hidden children (can be expanded) |
| `-` | Heading with visible children (can be collapsed) |
| `.` | Document reference (text file) |
| `&` | Cloned heading (twin/mate) |
| `?` | Create mode cursor indicator |
| (none) | Leaf node / final item without children |

---

## File Extensions

| Extension | File Type |
|-----------|-----------|
| `.CT` | THOUGHT Creative Thought outline file |
| `.DO` | Document file (text content) |
| `.TX` | Text file (alternative extension) |

---

## Platform Information

| Platform | Description |
|----------|-------------|
| Model 100 | Original TRS-80 Model 100 laptop |
| Model 200 | TRS-80 Model 200 with enhanced features |
| PC-8201a | Japanese variant of Model 100 |
| Virtual T | Emulator for testing THOUGHT on PC |

---

## Function Key Summary

| Mode | Key | Function |
|------|-----|----------|
| Create | ENTER | Create new entry / exit |
| Create | F8 | Exit to Review/Revise |
| Review | ↑↓←→ | Cursor movement |
| Review | F2 | Hide children |
| Review | F3 | Show children |
| Review | F4 | Clone |
| Review | F5 | Drag (move) |
| Review | F6 | Sort alphabetically |
| Review | F7 | Select range |
| Review | F8 | Exit to previous mode |
| Document | F6 | Enter document |
| Document | F8 | Exit to outline |
| Print | F1 | Go (print) |
| Print | F2 | Output destination |
| Print | F3 | Line spacing |
| Print | F4 | Page pause |
| Print | F5 | CRLF |
| Print | F6 | Set new defaults |
| Print | F7 | Advanced options |
| Print | F8 | Exit |