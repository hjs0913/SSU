USE [SSU_Project]
GO
/****** Object:  StoredProcedure [dbo].[save_player_info]    Script Date: 2022-07-27 오후 6:11:56 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[save_player_info] @Param0 NCHAR(10), @Param1 INT, @Param2 INT, @Param3 INT,@Param4 INT,@Param5 INT, @Param6 INT,@Param7 INT,@Param8 INT,@Param9 INT
	-- Add the parameters for the stored procedure here
	
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

    -- Insert statements for procedure here
	UPDATE SSU_Table Set player_x = @Param1, player_y = @Param2, player_z = @Param3, player_hp = @Param4,
	player_level = @Param5, player_exp = @Param6,player_maxhp = @Param7,player_mp = @Param8,player_maxmp = @Param9
	FROM SSU_Table WHERE player_id =  @param0;
END
