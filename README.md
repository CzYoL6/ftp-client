# ftp-client

一个简易的ftp客户端，实现的功能有：

- 下载远端文件
- 删除远端文件
- 上传本地文件

图形界面采用的`ImGui`

![image-20220526145122439](C:\Users\Lenovo\AppData\Roaming\Typora\typora-user-images\image-20220526145122439.png)



为了保持项目的简洁性，`boost`、`glfw`等头文件和库被直接放在了项目中，通过`cmake`组织。

程序运行需要`fonts`文件夹和`glfw3.dll`动态链接库。
