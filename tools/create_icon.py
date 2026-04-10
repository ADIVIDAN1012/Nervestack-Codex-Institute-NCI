from PIL import Image, ImageDraw
import os

def create_nervestack_icon():
    # Define size and colors
    size = (256, 256)
    bg_color = (13, 17, 23) # Dark distinctive blue (GitHub Dark Dimmed style)
    # nervestack_color = (0, 255, 255) # Cyan
    light_color = (255, 255, 255) # White
    beam_color_start = (56, 189, 248, 150) # Light blue transparent

    img = Image.new('RGBA', size, bg_color)
    draw = ImageDraw.Draw(img)

    # Calculate coordinates with more padding and perfect centering
    center_x = size[0] // 2
    center_y = size[1] // 2
    
    # Scale down by factor of 0.7 for safety margins
    scale = 0.7
    # Total drawing height approximation (beam top to base bottom)
    # Original: 120 (beam) + (216-60) base = ~280? No.
    # Base is from top_y to bottom_y.
    # Beams go up from top_y.
    # Let's define the bounding box of the drawing content.
    
    # Let's say total desired height of the icon graphics is 70% of canvas
    total_height = int(size[1] * scale)
    half_height = total_height // 2
    
    # We want the visual center of the object to be at center_y.
    # The object is: Base (trapezoid) + Light (circle) + Beams (arcs).
    # The "center" of the lighthouse is roughly the light position for visual balance?
    # Or just geometric center? Let's do geometric center of the main structure.
    
    # Let's place top_y and bottom_y relative to center
    # Top of base (where light is)
    # Bottom of base
    
    # Let's make the base height 60% of total height
    base_height = int(total_height * 0.6)
    
    # Let's align such that the middle of base is slightly below center to account for beams on top
    # Actually, simpler: define top_y and bottom_y
    
    top_y = center_y - int(base_height * 0.4) # Light slightly above center
    bottom_y = top_y + base_height
    
    # Re-center vertically if needed. 
    # Let's stick to the previous padding logic but fix the offsets.
    
    # Effective height of the main "Lighthouse" body (excluding beams potentially extending further up)
    # Previous: top_y = margin+40, bottom_y = size-margin-20.
    
    # New Logic:
    drawing_height = 180 # Fixed height for 256px canvas (approx 70%)
    top_y = center_y - (drawing_height // 2) + 20 # Shift down slightly so beams fit top
    bottom_y = top_y + 140 # Base height
    
    # Beams will stick up from top_y

    
    # Draw "Lighthouse" base (trapezoid)
    base_width_bottom = int(100 * scale)
    base_width_top = int(60 * scale)
    
    base_points = [
        (center_x - base_width_bottom // 2, bottom_y),
        (center_x + base_width_bottom // 2, bottom_y),
        (center_x + base_width_top // 2, top_y),
        (center_x - base_width_top // 2, top_y)
    ]
    draw.polygon(base_points, fill=(200, 200, 200)) #'#e5e7eb' gray-200

    # Draw "Light" (Circle at top)
    light_radius = int(25 * scale)
    light_box = [
        (center_x - light_radius, top_y - light_radius),
        (center_x + light_radius, top_y + light_radius)
    ]
    draw.ellipse(light_box, fill=light_color)

    # Draw "Beams" (Triangles radiating out)
    # Beams should not touch the absolute edge to avoid cropping
    beam_length = int(120 * scale)
    beam_spread = int(100 * scale) # spread at the end
    
    # We will just draw the concentric arcs as the "signal" as they look cleaner than a polygon beam
    # Adjust arc radii
    for r in [int(40*scale), int(60*scale), int(80*scale)]:
        draw.arc(
            [(center_x - r, top_y - r), (center_x + r, top_y + r)],
            start=220, end=320, fill=(0, 255, 255), width=5
        )

    # Save ICO for build
    assets_dir = os.path.join(os.getcwd(), 'assets')
    if not os.path.exists(assets_dir):
        os.makedirs(assets_dir)
    
    icon_path = os.path.join(assets_dir, 'nervestack_icon.ico')
    img.save(icon_path, format='ICO', sizes=[(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)])
    print(f"Icon created at {icon_path}")

    # Save PNG for website
    website_assets_dir = os.path.join(os.getcwd(), 'tools', 'website', 'assets')
    if not os.path.exists(website_assets_dir):
        os.makedirs(website_assets_dir)
        
    png_path = os.path.join(website_assets_dir, 'icon.png')
    img.save(png_path, format='PNG')
    print(f"Website icon created at {png_path}")

if __name__ == "__main__":
    create_nervestack_icon()
