let client = Redis.make();

client
|> Redis.set(~key="foo", ~value="bar", ~existence=NX, ~expiration=EX(10))
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(Redis.SimpleStringReply.Ok) => Js.log2("set", "Ok")
     | Belt.Result.Ok(Redis.SimpleStringReply.Empty) =>
       Js.log2("set", "Empty")
     | Belt.Result.Ok(Redis.SimpleStringReply.Unknown(value)) =>
       Js.log2("set", value)
     | Belt.Result.Error(error) => Js.log2("set error", error)
     }
   );

client
|> Redis.get(~key="foo")
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(Some(value)) => Js.log2("get", value)
     | Belt.Result.Ok(None) => Js.log2("get", "No value")
     | Belt.Result.Error(error) => Js.log2("get error", error)
     }
   );

client
|> Redis.del(~keys=[|"foo"|])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client |> Redis.quit;
