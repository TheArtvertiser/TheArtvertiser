Instructions:
-------------

copy contents of bin/ folder into The Artvertiser folder, cd to this dir then run:

$ hdiutil makehybrid -hfs -hfs-volume-name "The Artvertiser 0.92" -hfs-openfolder TheArtvertiser0.92dmg_staging TheArtvertiser0.92dmg_staging -o tmp.dmg
$ hdiutil convert -format UDZO tmp.dmg -o The\ Artvertiser\ 0.92.dmg
