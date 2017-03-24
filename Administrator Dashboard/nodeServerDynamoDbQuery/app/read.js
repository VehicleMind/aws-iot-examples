var AWS = require("aws-sdk");
var awsConfig = {
    "region": "ap-south-1",
    "endpoint": "http://dynamodb.ap-south-1.amazonaws.com",
    "accessKeyId": "AKIAJZMYM76N62LM3QDQ", "secretAccessKey": "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"
};
AWS.config.update(awsConfig);

var docClient = new AWS.DynamoDB.DocumentClient();
fetchOneByKey = function (name,city) {
    var params = {
        TableName: "sampletable",
        Key: {
            "name": name,
            "city": city
        }
    };
    docClient.get(params, function (err, data) {
        if (err) {
            console.log("users::fetchOneByKey::error - " + JSON.stringify(err, null, 2));
        }
        else {
            console.log(JSON.stringify(data, null, 2));
        }
    })
}
module.exports.fetchOneByKey = fetchOneByKey
