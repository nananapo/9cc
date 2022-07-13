[compilerbook](https://www.sigbus.info/compilerbook)を読みながら作っているCコンパイラ

動作環境

```sh
+zsh:1> uname -a
Darwin po.local 20.5.0 Darwin Kernel Version 20.5.0: Sat May  8 05:10:31 PDT 2021; root:xnu-7195.121.3~9/RELEASE_ARM64_T8101 x86_64
```

セルフホストのテスト
```sh
 make 9cc3 # コンパイラをセルフホスト
 make prpr3 # プリプロセッサをコンパイル
```

動く8queen

https://github.com/nananapo/9cc/blob/master/9cc/test/unit/9queen.c

動くDuff's device

https://github.com/nananapo/9cc/blob/master/9cc/test/unit/duffsdevice.c
