# 检查名字是否重复 https://docs.arduino.cc/libraries/

git tag -a 1.0.1
git push origin
git push origin --tags

################################# upload for platformio #################################

# 本地打包验证包含哪些文件
pio pkg pack

# 登录pio（可选）
pio account login

# 上传包
pio pkg publish --no-interactive

# 删除
pio pkg unpublish FastRotaryEncoder@1.0.1

# 验证是否上传成功
pio access list
# https://registry.platformio.org/
pio pkg search FastRotaryEncoder

################################# upload for arduino #################################

oepn https://github.com/fmeng/library-registry/
# sync fork
# 编辑 repositories.txt
# Commit changes https://github.com/fmeng/FastRotaryEncoder
# 本项目 Contribute -> Open pull request -> Create pull request
# ...
