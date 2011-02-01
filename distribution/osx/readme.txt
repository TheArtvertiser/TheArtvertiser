Instructions:
-------------

copy the build (contents of bin/ folder from oF project) into "TheArtvertiser0.92dmg_staging/The Artvertiser", cd into to this dir (osx/) then run:

$ hdiutil makehybrid -hfs -hfs-volume-name "The Artvertiser 0.92" -hfs-openfolder TheArtvertiser0.92dmg_staging TheArtvertiser0.92dmg_staging -o tmp.dmg
$ hdiutil convert -format UDZO tmp.dmg -o The\ Artvertiser\ 0.92.dmg
