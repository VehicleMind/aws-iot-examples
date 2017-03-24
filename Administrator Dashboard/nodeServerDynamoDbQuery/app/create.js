var AWS = require("aws-sdk");
var awsConfig = {
    "region": "ap-south-1",
    "endpoint": "http://dynamodb.ap-south-1.amazonaws.com",
    "accessKeyId": "AKIAJZMYM76N62LM3QDQ", "secretAccessKey": "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"
};
AWS.config.update(awsConfig);

var docClient = new AWS.DynamoDB.DocumentClient();

var save = function (name,city,food) {

    var input = {
        "name": name, "city": city,
        "food": food
    };
    var params = {
        TableName: "sampletable",
        Item:  input
    };
    docClient.put(params, function (err, data) {

        if (err) {
            console.log("users::save::error - " + JSON.stringify(err, null, 2));
        } else {
            console.log("users::save::success" );
        }
    });
}
module.exports.save = save
