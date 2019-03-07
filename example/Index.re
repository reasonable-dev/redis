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
|> Redis.exists(~key="foo")
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(true) => Js.log("exists")
     | Belt.Result.Ok(false) => Js.log("not exists")
     | Belt.Result.Error(error) => Js.log2("exists error", error)
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
|> Redis.scan(~cursor=Redis.Cursor.start, ~match="t*", ~count=1)
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok((cursor, results)) when Redis.Cursor.isLast(cursor) =>
       Js.log3("scan", "all results processed", results)
     | Belt.Result.Ok((cursor, results)) => Js.log3("scan", cursor, results)
     | Belt.Result.Error(error) => Js.log2("scan error", error)
     }
   );

client
|> Redis.del(~keys=["foo"])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client
|> Redis.hincrby(~key="foo", ~field="bar", ~value=1)
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("hincrby", value)
     | Belt.Result.Error(error) => Js.log2("hincrby error", error)
     }
   );

client
|> Redis.del(~keys=["foo"])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client
|> Redis.hmset(~key="foo", ~values=Js.Dict.fromList([("bar", "baz")]))
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(Redis.SimpleStringReply.Ok) => Js.log2("hmset", "Ok")
     | Belt.Result.Ok(Redis.SimpleStringReply.Empty) =>
       Js.log2("hmset", "Empty")
     | Belt.Result.Ok(Redis.SimpleStringReply.Unknown(value)) =>
       Js.log2("hmset", value)
     | Belt.Result.Error(error) => Js.log2("hmset error", error)
     }
   );

client
|> Redis.hscan(~key="foo", ~cursor=Redis.Cursor.start)
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok((cursor, results)) when Redis.Cursor.isLast(cursor) =>
       Js.log3("hscan", "all results processed", results)
     | Belt.Result.Ok((cursor, results)) => Js.log3("hscan", cursor, results)
     | Belt.Result.Error(error) => Js.log2("hscan error", error)
     }
   );

client
|> Redis.del(~keys=["foo"])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client
|> Redis.sadd(~key="foo", ~values=["one", "two", "three"])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("sadd", value)
     | Belt.Result.Error(error) => Js.log2("sadd error", error)
     }
   );

client
|> Redis.del(~keys=["foo"])
|> Redis.Promise.wait(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client |> Redis.quit;
