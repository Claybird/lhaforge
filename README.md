LhaForge2
===

LhaForge2 (LhaForge Ver.2.x.x)は、Windows用圧縮解凍ソフトです。圧縮解凍エンジンを内蔵しており、単体でも動作可能な圧縮解凍ソフトです。
また、圧縮ファイルを解凍せずに中身を確認したり、圧縮されたファイルが壊れていないかテストすることができます。
ショートカット経由だけではなく、エクスプローラのコンテキストメニュー(右クリックメニュー)や右ドラッグでのメニューにも対応し、幅広い使い方が可能です。

LhaForge2はMITライセンスで公開されており、個人利用・商用利用を問わず、無償で使用することが出来ます。

# 対応フォーマット

## 圧縮・解凍・テストの全てに対応

- zip
- tar
- gzip
- bz2
- zstandard
- lz4
- xz
- lzma
- 7z

## 解凍・テストのみ

- lzh
- zipx
- Microsoft Cabinet
- ISO9660 CD-ROM images
- rar
- arj
- cpio
- z(compress)
- uuencode
- bza/gza by [bga32.dll](https://www.madobe.net/archiver/lib/bga32.html)

## Ver.2.0.0以降、非対応

- 一部のCab形式
- [yz1](https://www.madobe.net/archiver/lib/yz1.html)
- [jak](https://www.madobe.net/archiver/lib/jack32.html)
- [ish](https://www.madobe.net/archiver/lib/aish32.html)
- [gca](https://www.madobe.net/archiver/lib/ungca32.html)
- [imp](https://www.madobe.net/archiver/lib/unimp32.html)
- [hki](https://www.madobe.net/archiver/lib/unhki32.html)
- [bel](https://www.madobe.net/archiver/lib/unbel32.html)
  
  

# 使用しているライブラリ

## 圧縮解凍エンジン

- [bzip2](https://sourceware.org/bzip2/downloads.html)
- [libarchive](https://github.com/libarchive/libarchive)
- [lz4](https://github.com/lz4/lz4)
- [minizip-ng](https://github.com/zlib-ng/minizip-ng)
- [zlib-ng](https://github.com/zlib-ng/zlib-ng.git)
- [unrar](https://github.com/Claybird/unrar); fork of https://www.rarlab.com/rar_add.htm
- [win_iconv](https://github.com/win-iconv/win-iconv)
- [zstd](https://github.com/facebook/zstd)
- [xz utils(liblzma)](https://git.tukaani.org/?p=xz.git;a=summary)

## ユーティリティ

- [libcharset](https://github.com/Claybird/libcharset-msvc); fork of https://www.gnu.org/software/libiconv/
- [simpleini](https://github.com/brofield/simpleini)
- [WTL(Windows Template Library)](https://sourceforge.net/projects/wtl/)

# 旧バージョンとの違い

旧バージョン(Ver.1.6.7およびそれ以前)では統合アーカイバプロジェクトのDLLを利用していましたが、現在のバージョン(LhaForge Ver.2.0.0以降)では使用しなくなりました。ほぼ全てのソースコードを書き換える、大幅な変更が加えられています。
これに伴い、従来サポートしていた形式の一部が非対応になったほか、アーカイバ「Noah」の機能拡張スクリプトB2Eへの対応も終了しています。
