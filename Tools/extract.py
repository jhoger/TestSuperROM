import os
from pdf2image import convert_from_path
import pytesseract

# Optional: Un-comment and set the path if Tesseract isn't in your system PATH (especially on Windows)
# pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'

def extract_text_from_pdf(pdf_path, output_txt_path):
    print("Converting PDF pages to images...")
    # Convert PDF pages to list of PIL Image objects
    # dpi=300 is recommended for clear text extraction
    pages = convert_from_path(pdf_path, dpi=300)
    
    extracted_text = []
    
    print(f"Starting OCR on {len(pages)} pages...")
    for i, page in enumerate(pages):
        print(f"Processing Page {i+1}...")
        # Extract text from the image page
        text = pytesseract.image_to_string(page)
        extracted_text.append(f"--- PAGE {i+1} ---\n{text}")
        
    # Save the aggregated text to a file
    with open(output_txt_path, "w", encoding="utf-8") as f:
        f.write("\n\n".join(extracted_text))
        
    print(f"Extraction complete! Saved to {output_txt_path}")

# Run the function
extract_text_from_pdf("Original Source Material/SuperRom-contents-and-overview.pdf", "overview.txt")
