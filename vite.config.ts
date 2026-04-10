import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  base: "/NCI/",
  build: {
    outDir: "dist",
    assetsDir: "assets",
  },
});
