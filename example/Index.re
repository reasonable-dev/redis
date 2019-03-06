let client = Redis.make();

client
|> Redis.set(~key="what", ~value="the", ~existence=NX, ~expiration=EX(1))
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

client |> Redis.get(~key="what") |> Redis.Promise.wait(Js.log2("get"));

client |> Redis.quit;
