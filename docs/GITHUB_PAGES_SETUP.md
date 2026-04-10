# Enable GitHub Pages for Nervestack Website

## ✅ Your code is now on GitHub!
Repository: https://github.com/ADIVIDAN1012/Nervestack-Codex-Institute-NCI

## Next Steps: Enable GitHub Pages

### 1. Go to Repository Settings
1. Open: https://github.com/ADIVIDAN1012/Nervestack-Codex-Institute-NCI
2. Click **Settings** (top right)
3. Click **Pages** (left sidebar)

### 2. Configure GitHub Pages
1. Under **Source**:
   - Branch: Select **main**
   - Folder: Select **/ (root)** or **/Nervestack-website**
2. Click **Save**

### 3. Wait for Deployment (1-2 minutes)
- GitHub will build and deploy your site
- You'll see a green checkmark when ready

### 4. Your Website Will Be Live At:
```
https://adividan1012.github.io/Nervestack-NCI/Nervestack-website/
```

Or if you selected root:
```
https://adividan1012.github.io/Nervestack-NCI/
```

## Optional: Custom Domain (Free)

If you want a custom domain later:

### Option 1: is-a.dev (Free Forever)
1. Fork: https://github.com/is-a-dev/register
2. Add `domains/Nervestacklang.json`:
```json
{
  "owner": {
    "username": "ADIVIDAN1012",
    "email": "aadityasadhu50@gmail.com"
  },
  "record": {
    "CNAME": "adividan1012.github.io"
  }
}
```
3. Submit PR
4. Once approved: `Nervestacklang.is-a.dev`

### Option 2: Keep GitHub Subdomain
- `adividan1012.github.io/Nervestack-NCI/Nervestack-website/`
- Completely free forever
- No setup needed

## Updating Your Website

Whenever you want to update the website:

```bash
cd c:\Users\aadit\OneDrive\Desktop\project_2_Nervestack

# Make changes to Nervestack-website/index.html or style.css

# Then push:
git add .
git commit -m "Update website"
git push origin main
```

GitHub will automatically redeploy within 1-2 minutes!

## Download Links

Once your site is live, update the download links in `index.html`:

```html
<!-- Change to GitHub releases -->
<a href="https://github.com/ADIVIDAN1012/Nervestack-Codex-Institute-NCI/releases/download/v1.0/Nervestack.exe">
    Download Compiler
</a>
```

To create a release:
1. Go to: https://github.com/ADIVIDAN1012/Nervestack-Codex-Institute-NCI/releases/new
2. Tag: `v1.0`
3. Title: `Nervestack v1.0`
4. Upload `Nervestack.exe` and `Nervestack-0.0.3.vsix`
5. Publish release

---

**Your website will be live in 1-2 minutes!** 🎉
