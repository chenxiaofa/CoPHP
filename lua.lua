--
-- Created by IntelliJ IDEA.
-- User: xiaofa
-- Date: 2016/7/11
-- Time: 14:42
-- To change this template use File | Settings | File Templates.
--

function a()
    print("a");
    b();
end

function b()
    print("b");
    coroutine.yield()
    print("c");
end

co = coroutine.create(function ()
    a();
end)



coroutine.resume(co)
print("d");
coroutine.resume(co)