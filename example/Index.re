let client = Redis.make();

Redis.set(~key="foo", ~value="bar", ~existence=NX, ~expiration=EX(10), client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(Redis.SimpleStringReply.Ok) => Js.log2("set", "Ok")
     | Belt.Result.Ok(Redis.SimpleStringReply.Empty) =>
       Js.log2("set", "Empty")
     | Belt.Result.Ok(Redis.SimpleStringReply.Unknown(value)) =>
       Js.log2("set", value)
     | Belt.Result.Error(error) => Js.log2("set error", error)
     }
   );

Redis.exists(~key="foo", client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(true) => Js.log("exists")
     | Belt.Result.Ok(false) => Js.log("not exists")
     | Belt.Result.Error(error) => Js.log2("exists error", error)
     }
   );

Redis.get(~key="foo", client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(Some(value)) => Js.log2("get", value)
     | Belt.Result.Ok(None) => Js.log2("get", "No value")
     | Belt.Result.Error(error) => Js.log2("get error", error)
     }
   );

Redis.scan(~cursor=Redis.Cursor.start, ~match="t*", ~count=1, client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok((cursor, results)) when Redis.Cursor.isLast(cursor) =>
       Js.log3("scan", "all results processed", results)
     | Belt.Result.Ok((cursor, results)) => Js.log3("scan", cursor, results)
     | Belt.Result.Error(error) => Js.log2("scan error", error)
     }
   );

Redis.del(~keys=["foo"], client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

Redis.hincrby(~key="foo", ~field="bar", ~value=1, client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("hincrby", value)
     | Belt.Result.Error(error) => Js.log2("hincrby error", error)
     }
   );

Redis.del(~keys=["foo"], client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

Redis.hmset(~key="foo", ~values=Js.Dict.fromList([("bar", "baz")]), client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(Redis.SimpleStringReply.Ok) => Js.log2("hmset", "Ok")
     | Belt.Result.Ok(Redis.SimpleStringReply.Empty) =>
       Js.log2("hmset", "Empty")
     | Belt.Result.Ok(Redis.SimpleStringReply.Unknown(value)) =>
       Js.log2("hmset", value)
     | Belt.Result.Error(error) => Js.log2("hmset error", error)
     }
   );

Redis.hscan(~key="foo", ~cursor=Redis.Cursor.start, client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok((cursor, results)) when Redis.Cursor.isLast(cursor) =>
       Js.log3("hscan", "all results processed", results)
     | Belt.Result.Ok((cursor, results)) => Js.log3("hscan", cursor, results)
     | Belt.Result.Error(error) => Js.log2("hscan error", error)
     }
   );

Redis.del(~keys=["foo"], client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

Redis.sadd(~key="foo", ~members=["one", "two", "three"], client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("sadd", value)
     | Belt.Result.Error(error) => Js.log2("sadd error", error)
     }
   );

Redis.scard(~key="foo", client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("scard", value)
     | Belt.Result.Error(error) => Js.log2("scard error", error)
     }
   );

Redis.sismember(~key="foo", ~member="one", client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("sismember", value)
     | Belt.Result.Error(error) => Js.log2("sismember error", error)
     }
   );

Redis.del(~keys=["foo"], client)
->Promise.get(res =>
     switch (res) {
     | Belt.Result.Ok(value) => Js.log2("del", value)
     | Belt.Result.Error(error) => Js.log2("del error", error)
     }
   );

client |> Redis.quit;
