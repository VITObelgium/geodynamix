rm -rf standalone
mkdir standalone
python build.py --dist --python-path=c:\python314\python.exe --build-dir=gdx-314
python build.py --dist --python-path=c:\python313\python.exe --build-dir=gdx-313
python build.py --dist --python-path=c:\python312\python.exe --build-dir=gdx-312

cp ./build/gdx-314-x64-windows-static-vs2022-dist/geodynamix.cp314-win_amd64.pyd ./standalone
cp ./build/gdx-313-x64-windows-static-vs2022-dist/geodynamix.cp313-win_amd64.pyd ./standalone
cp ./build/gdx-312-x64-windows-static-vs2022-dist/geodynamix.cp312-win_amd64.pyd ./standalone
